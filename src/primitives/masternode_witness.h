// Copyright (c) 2019-2020 The BITWIN24 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "util.h"
#include "../masternode.h"

class ActiveMasterNodeProofs;
bool operator==(const ActiveMasterNodeProofs& a, const ActiveMasterNodeProofs& b);
bool operator!=(const ActiveMasterNodeProofs& a, const ActiveMasterNodeProofs& b);

/** List proofs of active master nodes.  For validate reward in fresh block.
 *  Record will be removed later, for trusted blocks
 *  Miners must create it for each new block
 */
class CMasterNodeWitness
{
public:
    static const int32_t CURRENT_VERSION = 0;
    CAmount nVersion;
    uint32_t nTime;
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
        READWRITE(nTime);
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

    bool operator==(const CMasterNodeWitness &a)
    {
        if (nVersion != a.nVersion || nProofs.size() != a.nProofs.size()) {
            return false;
        }

        for (unsigned i = 0; i < nProofs.size(); ++i) {
            if (nProofs[i] != a.nProofs[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const CMasterNodeWitness &a)
    {
        return !(*this == a);
    }

    uint256 GetHash() const
    {
        CHashWriter ss(SER_GETHASH, 0);
        ss << nVersion;
        ss << nProofs;
        ss << nTargetBlockHash;
        ss << nTime;

        return ss.GetHash();
    }

    int CountMasterNodes()
    {
        return nProofs.size();
    }

    std::string ToString() const;
    bool Sign(CKey &keyWitness);
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
        return nPing.blockHash == 0 || nBroadcast.sigTime == 0;
    }

    friend bool operator==(const ActiveMasterNodeProofs& a, const ActiveMasterNodeProofs& b)
    {
        if (a.nVersion != b.nVersion
            || a.nPing != b.nPing
            || a.nBroadcast != b.nBroadcast) {
            return false;
        }
        return true;
    }
    friend bool operator!=(const ActiveMasterNodeProofs& a, const ActiveMasterNodeProofs& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};