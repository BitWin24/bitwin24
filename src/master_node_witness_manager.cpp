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

MasterNodeWitnessManager::MasterNodeWitnessManager()
    : CLevelDBWrapper(GetDataDir() / "mnwitness", 0, false, false), _lastUpdate(0), _threadRunning(false),
      _stopThread(false)
{
}

MasterNodeWitnessManager::~MasterNodeWitnessManager()
{
    _stopThread = true;
    while (_threadRunning) {
        MilliSleep(50);
    }
}

bool MasterNodeWitnessManager::Exist(const uint256 &targetBlockHash) const
{
    return _witnesses.find(targetBlockHash) != _witnesses.end();
}

bool MasterNodeWitnessManager::Add(const CMasterNodeWitness &proof, bool validate)
{
    boost::lock_guard<boost::mutex> guard(_mtx);
    if (!Exist(proof.nTargetBlockHash)) {
        if (!validate || proof.IsValid(GetAdjustedTime())) {
            LogPrint("MasterNodeWitnessManager", "Added proof %s\n", proof.ToString());
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
        LogPrint("MasterNodeWitnessManager", "Removed proof for target block %s\n", targetBlockHash.ToString());
        _witnesses.erase(targetBlockHash);
        return true;
    }
    return false;
}

void MasterNodeWitnessManager::UpdateThread()
{
    _stopThread = false;
    _threadRunning = true;
    while (!_stopThread) {
        MilliSleep(5000);

        if (GetAdjustedTime() - _lastUpdate > 5 * 60) {
            LogPrintf("MasterNodeWitnessManager::Update: begin remove old pings\n");
            _lastUpdate = GetAdjustedTime();

            int64_t thresholdTime = GetAdjustedTime() - MASTERNODE_REMOVAL_SECONDS;
            std::map<uint256, CMasterNodeWitness>::iterator it = _witnesses.begin();
            std::vector<uint256> toRemove;
            while (it != _witnesses.end()) {
                if (!it->second.IsValid(thresholdTime)) {
                    toRemove.push_back(it->first);
                }
                it++;
            }

            for (unsigned i = 0; i < toRemove.size(); i++) {
                Remove(toRemove[i]);
            }
            LogPrintf("MasterNodeWitnessManager::Update: finished removing old pings\n");
        }

        {
            boost::lock_guard<boost::mutex> guard(_mtx);
            const int WAITING_PROOFS_TIME = 30;
            for (int i = 0; i < _blocks.size(); i++) {
                CValidationState state;
                uint256 blockHash = _blocks[i].block.GetHash();
                if (Exist(blockHash) || (_blocks[i].creatingTime + WAITING_PROOFS_TIME) < GetAdjustedTime()) {
                    if (!mapBlockIndex.count(blockHash)) {
                        LogPrintf("try process new block %s, proof exist %d \n", blockHash.ToString(), Exist(blockHash));
                        CNode *pfrom = FindNode(_blocks[i].nodeID);
                        ProcessNewBlock(state, pfrom, &_blocks[i].block);
                        int nDoS;
                        if (state.IsInvalid(nDoS) && pfrom) {
                            CInv inv(MSG_BLOCK, blockHash);
                            string strCommand = "block";
                            pfrom->PushMessage("reject",
                                               strCommand,
                                               state.GetRejectCode(),
                                               state.GetRejectReason().substr(0, MAX_REJECT_MESSAGE_LENGTH),
                                               inv.hash);
                            if (nDoS > 0) {
                                TRY_LOCK(cs_main, lockMain);
                                if (lockMain) Misbehaving(pfrom->GetId(), nDoS);
                            }
                        }
                    }
                    _blocks.erase(_blocks.begin() + i);
                    i--;
                }
            }
        }
    }
    _threadRunning = false;
}

void MasterNodeWitnessManager::Save()
{
    boost::lock_guard<boost::mutex> guard(_mtx);
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

    std::map<std::pair<uint256, uint32_t>, CMasternodePing> pings;

    int64_t atTime = GetAdjustedTime();

    std::map<uint256, CMasternodePing>::iterator pingIt = mnodeman.mapSeenMasternodePing.begin();
    while (pingIt != mnodeman.mapSeenMasternodePing.end()) {
        const CMasternodePing &ping = pingIt->second;
        if (ping.sigTime<(atTime - MASTERNODE_REMOVAL_SECONDS) || ping.sigTime>(atTime + MASTERNODE_PING_SECONDS)) {
            pingIt++;
            continue;
        }
        std::pair<uint256, uint32_t> key(ping.vin.prevout.hash, ping.vin.prevout.n);
        if (pings.find(key) == pings.end() || pings[key].sigTime < ping.sigTime)
            pings[key] = ping;
        pingIt++;
    }

    std::vector<CTxIn> included;
    std::map<uint256, CMasternodeBroadcast>::iterator it = mnodeman.mapSeenMasternodeBroadcast.begin();
    while (it != mnodeman.mapSeenMasternodeBroadcast.end()) {
        std::pair<uint256, uint32_t> key(it->second.vin.prevout.hash, it->second.vin.prevout.n);
        if (pings.find(key) != pings.end()) {
            ActiveMasterNodeProofs proof;
            proof.nVersion = 0;
            proof.nBroadcast = it->second;
            proof.nPing = pings[key];
            if (std::find(included.begin(), included.end(), proof.nPing.vin) == included.end())
                result.nProofs.push_back(proof);
            included.push_back(proof.nPing.vin);
        }
        it++;
    }

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
    LogPrintf("Witness for target block hash not exist - %s\n", targetBlockHash.ToString());
    static CMasterNodeWitness result;
    return result;
}

void MasterNodeWitnessManager::HoldBlock(CBlock block, int nodeId)
{
    BlockInfo info;
    info.block = block;
    info.nodeID = nodeId;
    info.creatingTime = GetAdjustedTime();
    _blocks.push_back(info);
}