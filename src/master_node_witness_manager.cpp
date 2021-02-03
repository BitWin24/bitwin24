// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2019-2020 The BITWIN24 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "master_node_witness_manager.h"
#include "primitives/masternode_witness.h"
#include "masternodeman.h"
#include <exception>
#include "primitives/block.h"
#include "serialize.h"
#include "chainparams.h"
#include "obfuscation.h"
#include "main.h"


class Witness_StateCatcher : public CValidationInterface
{
public:
    uint256 hash;
    bool found;
    CValidationState state;

    Witness_StateCatcher(const uint256& hashIn) : hash(hashIn), found(false), state(){};

protected:
    virtual void BlockChecked(const CBlock& block, const CValidationState& stateIn)
    {
        if (block.GetHash() != hash)
            return;
        found = true;
        state = stateIn;
    };
};

MasterNodeWitnessManager::MasterNodeWitnessManager()
    : CLevelDBWrapper(GetDataDir() / "mnwitness", 0, false, false), _lastUpdate(0),
      _stopThread(false)
{
}

MasterNodeWitnessManager::~MasterNodeWitnessManager()
{
    _stopThread = true;
}

bool MasterNodeWitnessManager::Exist(const uint256 &targetBlockHash) const
{
    return _witnesses.find(targetBlockHash) != _witnesses.end();
}

bool MasterNodeWitnessManager::Add(const CMasterNodeWitness &proof, bool validate)
{
    boost::lock_guard<boost::mutex> guard(_mtx);
    for (auto& p : proof.nProofs) {
        mnodeman.mapSeenMasternodePingInsert(p.nPing.GetHash(), p.nPing);
    }
    if (!Exist(proof.nTargetBlockHash)) {
        if (!validate || proof.IsValid(GetAdjustedTime())) {
            _witnesses[proof.nTargetBlockHash] = proof;
            return true;
        }
    }
    return false;
}

bool MasterNodeWitnessManager::Remove(const uint256 &targetBlockHash)
{
    boost::lock_guard<boost::mutex> guard(_mtx);
    if (Exist(targetBlockHash)) {
        _witnesses.erase(targetBlockHash);
        return true;
    }
    return false;
}

void MasterNodeWitnessManager::UpdateThread()
{
    _stopThread = false;
    while (!_stopThread) {
        MilliSleep(2 * 60 * 1000);
        boost::lock_guard<boost::mutex> guard(_mtxGlobal);

        if (GetTime() - _lastUpdate > 5 * 60) {
            _lastUpdate = GetTime();

            int64_t thresholdTime = GetAdjustedTime() - 2 * MASTERNODE_REMOVAL_SECONDS;
            std::map<uint256, CMasterNodeWitness>::iterator it = _witnesses.begin();
            std::vector<uint256> toRemove;
            while (it != _witnesses.end()) {
                if (it->second.nTime < thresholdTime) {
                    toRemove.push_back(it->first);
                }
                it++;
            }

            for (unsigned i = 0; i < toRemove.size(); i++) {
                Remove(toRemove[i]);
            }
        }
    }
}

void MasterNodeWitnessManager::Save()
{
    boost::lock_guard<boost::mutex> guard(_mtxGlobal);
    EraseDB();
    std::map<uint256, CMasterNodeWitness>::iterator it = _witnesses.begin();
    while (it != _witnesses.end()) {
        Write(it->first, it->second);
        it++;
    }

    Sync();
    Flush();
}

void MasterNodeWitnessManager::Load()
{
    boost::lock_guard<boost::mutex> guard(_mtxGlobal);
    _witnesses.clear();

    try {
        boost::scoped_ptr<leveldb::Iterator> pcursor(NewIterator());
        pcursor->SeekToFirst();

        while (pcursor->Valid()) {
            try {
                leveldb::Slice slValue = pcursor->value();
                CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
                CMasterNodeWitness witness;
                ssValue >> witness;

                Add(witness);

                pcursor->Next();
            }
            catch (std::exception &e) {
                LogPrintf("%s : Deserialize or I/O error - %s\n", __func__, e.what());
            }
        }
    }
    catch (...) {
        LogPrintf("db for witnesses of master nodes not exist \n");
    }
}

CMasterNodeWitness MasterNodeWitnessManager::CreateMasterNodeWitnessSnapshot(uint256 targetBlockHash)
{
    CMasterNodeWitness result;
    result.nVersion = 0;
    result.nTargetBlockHash = targetBlockHash;
    result.nTime = GetAdjustedTime();

    std::map<std::pair<uint256, uint32_t>, CMasternodePing> pings;

    const auto t_begin = boost::chrono::high_resolution_clock::now();
    const auto mapSeenMasternodePing = mnodeman.mapSeenMasternodePingCopy();
    auto pingIt = mapSeenMasternodePing.begin();
    volatile int i = 0;
    while (pingIt != mapSeenMasternodePing.end()) {
        i++;
        const CMasternodePing &ping = pingIt->second;
        if (ping.sigTime < (result.nTime - MASTERNODE_REMOVAL_SECONDS) || ping.sigTime>(result.nTime + MASTERNODE_PING_SECONDS)) {
            pingIt++;
            continue;
        }
        std::pair<uint256, uint32_t> key(ping.vin.prevout.hash, ping.vin.prevout.n);
        if (pings.find(key) == pings.end() || pings[key].sigTime < ping.sigTime)
            pings[key] = ping;
        pingIt++;
    }

    const auto mapSeenMasternodeBroadcast = mnodeman.mapSeenMasternodeBroadcastCopy();
    std::vector<CTxIn> included;
    int skipped = 0;
    auto it = mapSeenMasternodeBroadcast.begin();
    while (it != mapSeenMasternodeBroadcast.end()) {
        std::pair<uint256, uint32_t> key(it->second.vin.prevout.hash, it->second.vin.prevout.n);
        if (pings.find(key) != pings.end()) {
            ActiveMasterNodeProofs proof;
            proof.nVersion = 0;
            proof.nBroadcast = it->second;
            proof.nPing = pings[key];
            bool skip = false;
            {
                CValidationState state;
                CMutableTransaction dummyTx = CMutableTransaction();
                CTxOut vout = CTxOut(2999.99 * COIN, obfuScationPool.collateralPubKey);
                dummyTx.vin.push_back(proof.nPing.vin);
                dummyTx.vout.push_back(vout);

                TRY_LOCK(cs_main, lockMain);
                if (lockMain && !AcceptableInputs(mempool, state, CTransaction(dummyTx), false, NULL)) {
                    skip = true;
                    skipped++;
                }
            }
            if (!skip && std::find(included.begin(), included.end(), proof.nPing.vin) == included.end()) {
                result.nProofs.push_back(proof);
            }
            else {

            }
            included.push_back(proof.nPing.vin);
        }
        it++;
    }
    const auto t_end = boost::chrono::high_resolution_clock::now();
    LogPrintf("CreateMasterNodeWitnessSnapshot time %d ms, masternode ping=%d, pings=%d, masternode broadcast=%d, skipped=%d, proofs=%d\n",
        boost::chrono::duration_cast<boost::chrono::milliseconds>(t_end - t_begin).count(),
        mapSeenMasternodePing.size(), pings.size(), mapSeenMasternodeBroadcast.size(), skipped, result.nProofs.size());

    return result;
}

void MasterNodeWitnessManager::EraseDB()
{
    std::vector<uint256> toRemove;
    try {
        boost::scoped_ptr<leveldb::Iterator> pcursor(NewIterator());
        pcursor->SeekToFirst();
        while (pcursor->Valid()) {
            leveldb::Slice slKey = pcursor->key();
            CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
            uint256 targetBlockHash;
            ssKey >> targetBlockHash;

            toRemove.push_back(targetBlockHash);

            pcursor->Next();
        }
    }
    catch (std::exception &e) {
        LogPrintf("%s : Deserialize or I/O error - %s\n", __func__, e.what());
    }

    for (std::vector<uint256>::iterator it = toRemove.begin(); it != toRemove.end(); it++) {
        uint256 target = *it;
        Erase(target, true);
    }
}

const CMasterNodeWitness &MasterNodeWitnessManager::Get(const uint256 &targetBlockHash)
{
    if (Exist(targetBlockHash))
        return _witnesses[targetBlockHash];
    static CMasterNodeWitness result;
    return result;
}

void MasterNodeWitnessManager::AddBroadCastToMNManager(const uint256 &targetBlockHash)
{
    if (!Exist(targetBlockHash)) {
        return;
    }
    CMasterNodeWitness witness = Get(targetBlockHash);
    for (auto it = witness.nProofs.begin(); it != witness.nProofs.end(); it++) {
        CMasternodeBroadcast mnb = (*it).nBroadcast;
        mnb.lastPing = (*it).nPing;

        if (mnodeman.mapSeenMasternodeBroadcastCount(mnb.GetHash()) &&
                mnodeman.Find(mnb.vin)) { //seen
            masternodeSync.AddedMasternodeList(mnb.GetHash());
            continue;
        }
        mnodeman.mapSeenMasternodeBroadcastInsert(mnb.GetHash(), mnb);

        int nDoS = 0;
        if (!mnb.CheckAndUpdate(nDoS)) {
            continue;
        }

        // make sure it's still unspent
        //  - this is checked later by .check() in many places and by ThreadCheckObfuScationPool()
        if (mnb.CheckInputsAndAdd(nDoS)) {
            masternodeSync.AddedMasternodeList(mnb.GetHash());
        }
    }
}