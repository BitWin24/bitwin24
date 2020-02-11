// Copyright (c) 2019-2020 The BITWIN24 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "masternode_witness.h"
#include "../util.h"

std::string CMasterNodeWitness::ToString() const
{
    std::stringstream s;
    s << strprintf("CMasterNodeWitness(target block hash=%s, ver=%d, count proofs=%d)\n",
                   nTargetBlockHash.ToString(),
                   nVersion,
                   nProofs.size());
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

bool CMasterNodeWitness::IsValid(int64_t atTime) const
{
    std::vector<CTxIn> checkedOut;
    for (unsigned i = 0; i < nProofs.size(); i++) {
        CMasternodePing ping = nProofs[i].nPing;
        CMasternodeBroadcast broadcast = nProofs[i].nBroadcast;

        if (ping.sigTime < (atTime - MASTERNODE_REMOVAL_SECONDS) || ping.sigTime > atTime) {
            return false;
        }

        if (ping.vin != broadcast.vin) {
            return false;
        }

        if (!broadcast.VerifySignature()) {
            return false;
        }

        int nDos = 0;
        if (!ping.VerifySignature(broadcast.pubKeyMasternode, nDos) || nDos != 0) {
            return false;
        }

        uint256 hashBlock = 0;
        CTransaction tx2;
        GetTransaction(ping.vin.prevout.hash, tx2, hashBlock, true);
        BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end() && (*mi).second) {
            CBlockIndex *pMNIndex = (*mi).second;
            CBlockIndex *pConfIndex = chainActive[pMNIndex->nHeight + MASTERNODE_MIN_CONFIRMATIONS - 1];
            if (pConfIndex->GetBlockTime() > atTime) {
                return false;
            }
        }

        if (std::find(checkedOut.begin(), checkedOut.end(), ping.vin) != checkedOut.end()) {
            return false;
        }
        checkedOut.push_back(ping.vin);
    }
    return true;
}

bool CMasterNodeWitness::SignatureValid() const
{
    CPubKey pubkey;
    if (!pubkey.RecoverCompact(GetHash(), vchSig)) {
        return false;
    }
    return (pubkey.GetID() == pubKeyWitness.GetID());
}

std::string ActiveMasterNodeProofs::ToString() const
{
    std::stringstream s;
    s << strprintf("\tActiveMasterNodeProofs ver=%d\n", nVersion);
    s << "\tPing " << nPing.blockHash.ToString() << " " << strprintf("sigTime %s", EpochTimeToHumanReadableFormat(nPing.sigTime)) << "\n";
    s << "\tBroadcast " << nBroadcast.addr.ToString() << " " << nBroadcast.vin.ToString() << "\n";
    return s.str();
}