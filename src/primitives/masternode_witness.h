// Copyright (c) 2019-2020 The BITWIN24 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "util.h"
#include "../masternode.h"

class ActiveMasterNodeProofs;

/** List proofs of active master nodes.  For validate reward in fresh block.
 *  Record will be removed later, for trusted blocks
 *  Miners must create it for each new block
 */
class CMasterNodeWitness
{
public:
    static const int32_t CURRENT_VERSION = 0;
    CAmount nVersion;
    uint256 nTargetBlockHash;
    std::vector<ActiveMasterNodeProofs> nProofs;
    std::vector<unsigned char> vchSig;
    CPubKey pubKeyWitness;

    CMasterNodeWitness()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template<typename Stream, typename Operation>
    inline void SerializationOp(Stream &s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(this->nVersion);
        READWRITE(nProofs);
        READWRITE(nTargetBlockHash);
        READWRITE(pubKeyWitness);
        READWRITE(vchSig);
    }

    void SetNull()
    {
        nVersion = -1;
    }

    bool IsNull() const
    {
        return nVersion == -1;
    }

    void SetEmpty()
    {
        nVersion = CURRENT_VERSION;
        nProofs.clear();
    }

    bool IsEmpty() const
    {
        return nProofs.empty();
    }

    bool operator==(const CMasterNodeWitness &a, const CMasterNodeWitness &b)
    {
        if (a.nVersion != b.nVersion || a.nProofs.size() != b.nProofs.size())
            return false;

        for (int i = 0; i < a.nProofs.size(); ++i)
            if (a.nProofs[i] != b.nProofs[i])
                return false;

        return true;
    }

    bool operator!=(const CMasterNodeWitness &a, const CMasterNodeWitness &b)
    {
        return !(a == b);
    }

    uint256 GetHash()
    {
        CHashWriter ss(SER_GETHASH, 0);
        ss << nVersion;
        ss << nProofs;
        ss << nTargetBlockHash;

        return ss.GetHash();
    }

    int CountMasterNodes()
    {
        return nProofs.size();
    }

    std::string ToString() const;
    bool Sign(CKey &keyWitness, CPubKey &pubKeyWitness);
    bool IsValid(int64_t atTime) const;
    bool SignatureValid() const;
};

/** Proof of active masternode, must contains ping and masternode broadcast
 */
class ActiveMasterNodeProofs
{
public:
    static const int32_t CURRENT_VERSION = 0;
    CAmount nVersion;
    CMasternodePing nPing;
    CMasternodeBroadcast nBroadcast;

    ActiveMasterNodeProofs()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template<typename Stream, typename Operation>
    inline void SerializationOp(Stream &s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(this->nVersion);
        READWRITE(nPing);
        READWRITE(nBroadcast);
    }

    void SetNull()
    {
        SetEmpty();
        nVersion = -1;
    }

    bool IsNull() const
    {
        return nVersion == -1;
    }

    void SetEmpty()
    {
        nVersion = CURRENT_VERSION;
        nPing = CMasternodePing();
        nBroadcast = CMasternodeBroadcast();
    }

    bool IsEmpty() const
    {
        return nPing.vchSig.empty() || nBroadcast.nPing.empty();
    }

    bool operator==(const CMasterNodeWitness &a, const CMasterNodeWitness &b)
    {
        if (a.nVersion != b.nVersion || a.nProofs.size() != b.nProofs.size())
            return false;

        for (int i = 0; i < a.nProofs.size(); ++i)
            if (a.nProofs[i] != b.nProofs[i])
                return false;

        return true;
    }

    bool operator!=(const CMasterNodeWitness &a, const CMasterNodeWitness &b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};