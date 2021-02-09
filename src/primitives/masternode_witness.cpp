// Copyright (c) 2019-2020 The BITWIN24 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "masternode_witness.h"
#include "../util.h"
#include "../obfuscation.h"

std::string CMasterNodeWitness::ToString() const
{
    std::stringstream s;
    s << strprintf("CMasterNodeWitness(target time=%s block hash=%s, ver=%d, count proofs=%d)\n",
                   EpochTimeToHumanReadableFormat(nTime),
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

        if (ping.sigTime<(atTime - MASTERNODE_REMOVAL_SECONDS) || ping.sigTime>(atTime + MASTERNODE_PING_SECONDS)) {
            LogPrintf("DEBUG: invalid ping time, ping.sigTime=%d, atTime=%d\n", ping.sigTime, atTime);
            return false;
        }

        if (ping.vin != broadcast.vin) {
            LogPrintf("DEBUG: vin mismatch %s != %s\n", ping.vin.ToString(), broadcast.vin.ToString());
            return false;
        }

        if (!broadcast.VerifySignature()) {
            LogPrintf("DEBUG: !broadcast.VerifySignature()\n");
            return false;
        }

        int nDos = 0;
        if (!ping.VerifySignature(broadcast.pubKeyMasternode, nDos) || nDos != 0) {
            LogPrintf("DEBUG: !ping.VerifySignature(broadcast.pubKeyMasternode, nDos) || nDos != 0\n");
            return false;
        }

        uint256 hashBlock = 0;
        CTransaction tx2;
        GetTransaction(ping.vin.prevout.hash, tx2, hashBlock, true);
        BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end() && (*mi).second) {
            CBlockIndex *pMNIndex = (*mi).second;
            CBlockIndex *pConfIndex = chainActive[pMNIndex->nHeight + MASTERNODE_MIN_CONFIRMATIONS - 1];
            if (pConfIndex && pConfIndex->GetBlockTime() > atTime) {
            LogPrintf("DEBUG: pConfIndex && pConfIndex->GetBlockTime() (%d) > atTime (%d)\n", pConfIndex->GetBlockTime(), atTime);
                return false;
            }
        }

        // check that Master node vin\vout is not spent
        {
            CValidationState state;
            CMutableTransaction dummyTx = CMutableTransaction();
            CTxOut vout = CTxOut(2999.99 * COIN, obfuScationPool.collateralPubKey);
            dummyTx.vin.push_back(ping.vin);
            dummyTx.vout.push_back(vout);

            LOCK(cs_main);
            if (!AcceptableInputs(mempool, state, CTransaction(dummyTx), false, NULL)) {
                LogPrintf("DEBUG: bad input; vin: %s\n", ping.vin.ToString());
            }
        }

        if (std::find(checkedOut.begin(), checkedOut.end(), ping.vin) != checkedOut.end()) {
            LogPrintf("DEBUG: std::find(checkedOut.begin(), checkedOut.end(), ping.vin) != checkedOut.end()\n");
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
    s << "\tPing " << nPing.vin.ToString() << " "
      << strprintf("sigTime %s", EpochTimeToHumanReadableFormat(nPing.sigTime).c_str()) << "\n";
    s << "\tBroadcast " << nBroadcast.addr.ToString() << " " << nBroadcast.vin.ToString()
      << EpochTimeToHumanReadableFormat(nBroadcast.sigTime).c_str() << "\n";
    return s.str();
}