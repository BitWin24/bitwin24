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

bool CMasterNodeWitness::Sign(CKey &keyWitness)
{
    std::string errorMessage;

    uint256 hash = GetHash();

    if (!keyWitness.SignCompact(hash, vchSig)) {
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

        if (ping.sigTime < (atTime - MASTERNODE_REMOVAL_SECONDS) || ping.sigTime > (atTime + MASTERNODE_PING_SECONDS)) {
            LogPrintf("atTime %s ping.sigTime %s ping.vin.prevout.hash %s \n", atTime, ping.sigTime, ping.vin.prevout.hash.ToString());
            return false;
        }

        if (ping.vin != broadcast.vin) {
            LogPrintf("ping.vin != broadcast.vin \n");
            return false;
        }

        if (!broadcast.VerifySignature()) {
            LogPrintf("broadcast.VerifySignature \n");
            return false;
        }

        int nDos = 0;
        if (!ping.VerifySignature(broadcast.pubKeyMasternode, nDos) || nDos != 0) {
            LogPrintf("ping.VerifySignature \n");
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
                LogPrintf("pConfIndex->GetBlockTime() > atTime \n");
                return false;
            }
        }

        if (std::find(checkedOut.begin(), checkedOut.end(), ping.vin) != checkedOut.end()) {
            LogPrintf("checkedOut.begin(), checkedOut.end \n");
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
    s << "\tPing " << nPing.vin.ToString() << " " << strprintf("sigTime %s", EpochTimeToHumanReadableFormat(nPing.sigTime)) << "\n";
    s << "\tBroadcast " << nBroadcast.addr.ToString() << " " << nBroadcast.vin.ToString() << "\n";
    return s.str();
}