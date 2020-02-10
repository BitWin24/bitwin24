// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2019-2020 The BITWIN24 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "master_node_witness_manager.h"
#include "primitives/masternode_witness.h"
#include "masternodeman.h"

MasterNodeWitnessManager::MasterNodeWitnessManager()
{
}

bool MasterNodeWitnessManager::Exist(const uint256 &targetBlockHash) const
{
    return _witnesses.find(targetBlockHash) != _witnesses.end();
}

bool MasterNodeWitnessManager::Add(const CMasterNodeWitness &proof, bool validate)
{
    if (!exist(proof.nTargetBlockHash)) {
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
    if (exist(targetBlockHash)) {
        LogPrint("MasterNodeWitnessManager", "Removed proof for target block %s\n", targetBlockHash.ToString());
        _witnesses.erase(targetBlockHash);
        return true;
    }
    return false;
}

void MasterNodeWitnessManager::Update()
{
    if (GetAdjustedTime() - _lastUpdate < 5 * 60 * 1000) {
        return;
    }
    _lastUpdate = GetAdjustedTime();

    int64_t thresholdTime = GetAdjustedTime() - MASTERNODE_REMOVAL_SECONDS;
    std::map<uint256, CMasterNodeWitness>::iterator it = _witnesses.begin();
    std::vector<uint256> toRemove;
    while (it != _witnesses.end()) {
        if (!it->second->IsValid(thresholdTime)) {
            toRemove.push(it->first);
        }
        it++;
    }

    for (int i = 0; i < toRemove.size(); i++) {
        Remove(toRemove[i]);
    }
}

void MasterNodeWitnessManager::Save()
{
}

void MasterNodeWitnessManager::load()
{
}

const CMasterNodeWitness &MasterNodeWitnessManager::find(const uint256 &targetBlockHash)
{
    if (exist(targetBlockHash))
        return _witnesses[targetBlockHash];
    return CMasterNodeWitness();
}

CMasterNodeWitness MasterNodeWitnessManager::CreateMasterNodeWitnessSnapshot(uint256 targetBlockHash)
{
    CMasterNodeWitness result;
    result.nVersion = 0;
    result.nTargetBlockHash = targetBlockHash;

    std::map<uint256, CMasternodeBroadcast>::iterator it = mnodeman.mapSeenMasternodeBroadcast.begin();
    std::vector<uint256> toRemove;
    while (it != mnodeman.mapSeenMasternodeBroadcast.end()) {
        if (!it->second->IsValid(thresholdTime)) {
            ActiveMasterNodeProofs proof;
            proof.nVersion = 0;
            proof.nBroadcast = it->second;
            proof.nPing = proof.nBroadcast.lastPing;
            result.nProofs.push(proof);
        }
        it++;
    }

    return result;
}