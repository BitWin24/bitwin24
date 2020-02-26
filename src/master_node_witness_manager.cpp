#include <trace-log.h> //++++++++++++++++++
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

MasterNodeWitnessManager::MasterNodeWitnessManager()
    : CLevelDBWrapper(GetDataDir() / "mnwitness", 0, false, false), _lastUpdate(0),
      _stopThread(false)
{
}

MasterNodeWitnessManager::~MasterNodeWitnessManager()
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    _stopThread = true;
}

bool MasterNodeWitnessManager::Exist(const uint256 &targetBlockHash) const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    return _witnesses.find(targetBlockHash) != _witnesses.end();
}

bool MasterNodeWitnessManager::Add(const CMasterNodeWitness &proof, bool validate)
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    boost::lock_guard<boost::mutex> guard(_mtx);
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

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    boost::lock_guard<boost::mutex> guard(_mtx);
    if (Exist(targetBlockHash)) {
        _witnesses.erase(targetBlockHash);
        return true;
    }
    return false;
}

void MasterNodeWitnessManager::UpdateThread()
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    _stopThread = false;
    while (!_stopThread) {
        MilliSleep(5000);
        boost::lock_guard<boost::mutex> guard(_mtxGlobal);

        if (GetTime() - _lastUpdate > 5 * 60) {
            _lastUpdate = GetTime();

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
        }

        {

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
            boost::lock_guard<boost::mutex> guard(_mtx);
            std::vector<uint256> toRemove;
            for (auto it = _blocks.begin(); it != _blocks.end(); it++) {
                CValidationState state;
                const CBlock &block = it->second.block;
                uint256 blockHash = block.GetHash();
                if (Exist(blockHash)
                    || _retries[blockHash]._retry > 4
                    || chainActive.Tip()->nHeight < START_HEIGHT_REWARD_BASED_ON_MN_COUNT) {
                    if (!mapBlockIndex.count(blockHash)) {
                        if (!mapBlockIndex.count(block.hashPrevBlock)) {
                            continue;
                        }
                        CInv inv(MSG_BLOCK, blockHash);
                        CNode *pfrom = FindNode(it->second.nodeID);
                        if(!pfrom)
                            LogPrintf("received block from unknown peer %d", it->second.nodeID);
                        if(pfrom)
                            pfrom->AddInventoryKnown(inv);
                        ProcessNewBlock(state, pfrom, &it->second.block);
                        int nDoS;
                        if (state.IsInvalid(nDoS) && pfrom) {
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
                        toRemove.push_back(it->first);
                    }
                }
                else if ((_retries[blockHash]._lastTryTime + 5) < GetTime()) {
                    _retries[blockHash]._lastTryTime = GetTime();
                    _retries[blockHash]._retry++;
                    BOOST_FOREACH(CNode * pnode, vNodes)
                        if (pnode->nVersion >= MASTER_NODE_WITNESS_VERSION)
                            pnode->PushMessage("getmnwitness", block.GetHash());
                }
            }

            for (auto it = toRemove.begin(); it != toRemove.end(); it++) {
                _blocks.erase(*it);
            }
        }
    }
}

void MasterNodeWitnessManager::Save()
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
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

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
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

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
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
            bool skip = false;
//            {
//                CValidationState state;
//                CMutableTransaction dummyTx = CMutableTransaction();
//                CTxOut vout = CTxOut(2999.99 * COIN, obfuScationPool.collateralPubKey);
//                dummyTx.vin.push_back(proof.nPing.vin);
//                dummyTx.vout.push_back(vout);
//
//                TRY_LOCK(cs_main, lockMain);
//                if (lockMain && !AcceptableInputs(mempool, state, CTransaction(dummyTx), false, NULL)) {
//                    skip = true;
//                }
//            }
            if (!skip && std::find(included.begin(), included.end(), proof.nPing.vin) == included.end())
                result.nProofs.push_back(proof);
            included.push_back(proof.nPing.vin);
        }
        it++;
    }

    return result;
}

void MasterNodeWitnessManager::EraseDB()
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
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

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    if (Exist(targetBlockHash))
        return _witnesses[targetBlockHash];
    static CMasterNodeWitness result;
    return result;
}

void MasterNodeWitnessManager::HoldBlock(CBlock block, int nodeId)
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    if (!_blocks.count(block.GetHash())) {
        BlockInfo info;
        info.block = block;
        info.nodeID = nodeId;
        info.creatingTime = GetTime();
        _blocks[block.GetHash()] = info;
        RETRY_REQUEST retry;
        retry._retry = 0;
        retry._lastTryTime = GetTime();
        _retries[block.GetHash()] = retry;
    }
}