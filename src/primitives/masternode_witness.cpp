#include "masternode_witness.h"

std::string CMasterNodeWitness::ToString() const
{
    std::stringstream s;
    s << strprintf("CMasterNodeWitness(target block hash=%s, ver=%d, count proofs=%d, removed=%d)\n",
                   nTargetBlockHash.ToString(),
                   nVersion,
                   nProofs.size(),
                   nRemoved);
    for (unsigned int i = 0; i < nProofs.size(); i++)
    {
        s << "  " << nProofs[i].ToString() << "\n";
    }
    return s.str();
}

std::string ActiveMasterNodeProofs::ToString() const
{
    std::stringstream s;
    s << strprintf("ActiveMasterNodeProofs(ver=%d\n", nVersion);
    s << "  " << nPing.ToString() << "\n";
    s << "  " << nBroadcast.ToString() << "\n";
    return s.str();
}