// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2019-2020 The BITWIN24 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "masternode_witness.h"

std::string CMasterNodeWitness::ToString() const
{
    std::stringstream s;
    s << strprintf("CMasterNodeWitness(target block hash=%s, ver=%d, count proofs=%d, removed=%d, signature=%s)\n",
                   nTargetBlockHash.ToString(),
                   nVersion,
                   nProofs.size(),
                   nRemoved,
                   nSignature.ToString());
    for (unsigned int i = 0; i < nProofs.size(); i++) {
        s << "  " << nProofs[i].ToString() << "\n";
    }
    return s.str();
}

bool CMasterNodeWitness::Sign(CKey &keyWitness, CPubKey &pubKeyWitness)
{
    std::string errorMessage;

    uint256 hash = GetHash();

    if (!keyWitness.Sign(hash, vchSig)) {
        LogPrint("witness", "CMasterNodeWitness::Sign() - Can't sign\n");
        return false;
    }
    return true;
}

std::string CMasterNodeWitness::IsValid(int64_t atTime) const
{
    return true;
}

std::string CMasterNodeWitness::SignatureValid() const
{
    CPubKey pubkey;
    if (!pubkey.RecoverCompact(GetHash(), vchSig)) {
        errorMessage = _("Error recovering public key.");
        return false;
    }
    return (pubkey.GetID() == pubKeyWitness.GetID());
}

std::string ActiveMasterNodeProofs::ToString() const
{
    std::stringstream s;
    s << strprintf("ActiveMasterNodeProofs(ver=%d\n", nVersion);
    s << "  " << nPing.ToString() << "\n";
    s << "  " << nBroadcast.ToString() << "\n";
    return s.str();
}