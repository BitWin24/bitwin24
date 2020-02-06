// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2019-2020 The BITWIN24 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "master_node_witness_manager.h"
#include "primitives/masternode_witness.h"

MasterNodeWitnessManager::MasterNodeWitnessManager()
{
}

bool MasterNodeWitnessManager::exist(const uint256 &targetBlockHash) const
{
    return _witnesses.find(targetBlockHash) != _witnesses.end();
}

bool MasterNodeWitnessManager::add(const CMasterNodeWitness &proof)
{
    if (!exist(proof.nTargetBlockHash)) {
        if (!proof.IsValid(0)) {
            LogPrint("MasterNodeWitnessManager", "Added proof %s\n", proof.ToString());
            _witnesses[proof.nTargetBlockHash] = proof;
            return true;
        }
    }
    return false;
}

bool MasterNodeWitnessManager::remove(const CMasterNodeWitness &proof)
{
    if (exist(proof.nTargetBlockHash)) {
        LogPrint("MasterNodeWitnessManager", "Removed proof %s\n", proof.ToString());
        _witnesses.erase(proof.nTargetBlockHash);
        return true;
    }
    return false;
}

bool MasterNodeWitnessManager::isRemoved(const uint256 &targetBlockHash)
{
    return exist(targetBlockHash) && _witnesses[targetBlockHash].nRemoved;
}

void MasterNodeWitnessManager::update()
{
}

void MasterNodeWitnessManager::save()
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
