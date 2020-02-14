// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2019-2020 The BITWIN24 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "leveldbwrapper.h"
#include "base58.h"

#include <unordered_set>
#include <string>

class CMasterNodeWitness;
class MasterNodeWitnessManager;

/*
 * Contains proofs of active master nodes
 */
class MasterNodeWitnessManager: public CLevelDBWrapper
{
public:
    MasterNodeWitnessManager();

    bool Exist(const uint256 &targetBlockHash) const;
    bool Add(const CMasterNodeWitness &proof, bool validate = false);
    bool Remove(const uint256 &targetBlockHash);
    const CMasterNodeWitness &Get(const uint256 &targetBlockHash);
    CMasterNodeWitness CreateMasterNodeWitnessSnapshot(uint256 targetBlockHash = 0);

    void Update();
    void Save();
    void Load();
private:
    void EraseDB();
    std::map<uint256, CMasterNodeWitness> _witnesses;
    int64_t _lastUpdate;
};