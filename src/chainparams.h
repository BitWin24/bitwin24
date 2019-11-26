#include "trace-log.h" //++++++++++++++++++
// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX developers
// Copyright (c) 2018 The MAC developers
// Copyright (c) 2019 The BITWIN24 developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHAINPARAMS_H
#define BITCOIN_CHAINPARAMS_H

#include "chainparamsbase.h"
#include "checkpoints.h"
#include "primitives/block.h"
#include "protocol.h"
#include "uint256.h"

#include "libzerocoin/Params.h"
#include <vector>

typedef unsigned char MessageStartChars[MESSAGE_START_SIZE];

struct CDNSSeedData {
    std::string name, host;
    CDNSSeedData(const std::string& strName, const std::string& strHost) : name(strName), host(strHost) {}
};

/**
 * CChainParams defines various tweakable parameters of a given instance of the
 * BITWIN24 system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class CChainParams
{
public:
    enum Base58Type {
        PUBKEY_ADDRESS,
        SCRIPT_ADDRESS,
        SECRET_KEY,     // BIP16
        EXT_PUBLIC_KEY, // BIP32
        EXT_SECRET_KEY, // BIP32
        EXT_COIN_TYPE,  // BIP44

        MAX_BASE58_TYPES
    };

    const uint256& HashGenesisBlock() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return hashGenesisBlock; }
    const MessageStartChars& MessageStart() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return pchMessageStart; }
    const std::vector<unsigned char>& AlertKey() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return vAlertPubKey; }
    int GetDefaultPort() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nDefaultPort; }
    const uint256& ProofOfWorkLimit() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return bnProofOfWorkLimit; }
    int SubsidyHalvingInterval() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nSubsidyHalvingInterval; }
    /** Used to check majorities for block version upgrade */
    int EnforceBlockUpgradeMajority() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nEnforceBlockUpgradeMajority; }
    int RejectBlockOutdatedMajority() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nRejectBlockOutdatedMajority; }
    int ToCheckBlockUpgradeMajority() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nToCheckBlockUpgradeMajority; }
    int MaxReorganizationDepth() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMaxReorganizationDepth; }

    /** Used if GenerateBitcoins is called with a negative number of threads */
    int DefaultMinerThreads() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMinerThreads; }
    const CBlock& GenesisBlock() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return genesis; }
    /** Make miner wait to have peers to avoid wasting work */
    bool MiningRequiresPeers() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return fMiningRequiresPeers; }
    /** Headers first syncing is disabled */
    bool HeadersFirstSyncingActive() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return fHeadersFirstSyncingActive; };
    /** Default value for -checkmempool and -checkblockindex argument */
    bool DefaultConsistencyChecks() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return fDefaultConsistencyChecks; }
    /** Allow mining of a min-difficulty block */
    bool AllowMinDifficultyBlocks() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return fAllowMinDifficultyBlocks; }
    /** Skip proof-of-work check: allow mining of any difficulty block */
    bool SkipProofOfWorkCheck() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return fSkipProofOfWorkCheck; }
    /** Make standard checks */
    bool RequireStandard() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return fRequireStandard; }
    int64_t TargetTimespan() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nTargetTimespan; }
    int64_t TargetSpacing() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nTargetSpacing; }
    int64_t Interval() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nTargetTimespan / nTargetSpacing; }
    int COINBASE_MATURITY() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMaturity; }
    CAmount MaxMoneyOut() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMaxMoneyOut; }
    /** The masternode count that we will allow the see-saw reward payments to be off by */
    int MasternodeCountDrift() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMasternodeCountDrift; }
    /** Make miner stop after a block is found. In RPC, don't return until nGenProcLimit blocks are generated */
    bool MineBlocksOnDemand() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return fMineBlocksOnDemand; }
    /** In the future use NetworkIDString() for RPC fields */
    bool TestnetToBeDeprecatedFieldRPC() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return fTestnetToBeDeprecatedFieldRPC; }
    /** Return the BIP70 network string (main, test or regtest) */
    std::string NetworkIDString() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return strNetworkID; }
    const std::vector<CDNSSeedData>& DNSSeeds() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return vSeeds; }
    const std::vector<unsigned char>& Base58Prefix(Base58Type type) const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return base58Prefixes[type]; }
    const std::vector<CAddress>& FixedSeeds() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return vFixedSeeds; }
    virtual const Checkpoints::CCheckpointData& Checkpoints() const = 0;
    int PoolMaxTransactions() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nPoolMaxTransactions; }
    bool ZeroCoinEnabled() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return fZeroCoinEnabled; }

    /** Spork key and Masternode Handling **/
    std::string SporkKey() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return strSporkKey; }
    std::string SporkKeyOld() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return strSporkKeyOld; }
    int64_t NewSporkStart() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nEnforceNewSporkKey; }
    int64_t RejectOldSporkKey() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nRejectOldSporkKey; }
    std::string ObfuscationPoolDummyAddress() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return strObfuscationPoolDummyAddress; }
    int64_t StartMasternodePayments() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nStartMasternodePayments; }
    int64_t Budget_Fee_Confirmations() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nBudget_Fee_Confirmations; }

    CBaseChainParams::Network NetworkID() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return networkID; }

    /** Zerocoin **/
    std::string Zerocoin_Modulus() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return zerocoinModulus; }
    libzerocoin::ZerocoinParams* Zerocoin_Params(bool useModulusV1) const;
    int Zerocoin_MaxSpendsPerTransaction() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMaxZerocoinSpendsPerTransaction; }
    CAmount Zerocoin_MintFee() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMinZerocoinMintFee; }
    int Zerocoin_MintRequiredConfirmations() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMintRequiredConfirmations; }
    int Zerocoin_RequiredAccumulation() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nRequiredAccumulation; }
    int Zerocoin_DefaultSpendSecurity() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nDefaultSecurityLevel; }
    int Zerocoin_HeaderVersion() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nZerocoinHeaderVersion; }
    int Zerocoin_RequiredStakeDepth() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nZerocoinRequiredStakeDepth; }

    int64_t SwapAmount() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nSwapAmount; }
    int64_t SwapPoWBlocks() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nSwapPoWBlocks; }
    int64_t SwapCoinbaseValue() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nSwapCoinbaseValue; }

    /** Height or Time Based Activations **/
    int ModifierUpgradeBlock() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nModifierUpdateBlock; }
    int LAST_POW_BLOCK() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nLastPOWBlock; }
    int Zerocoin_StartHeight() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nZerocoinStartHeight; }
    int Zerocoin_Block_EnforceSerialRange() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nBlockEnforceSerialRange; }
    int Zerocoin_Block_RecalculateAccumulators() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nBlockRecalculateAccumulators; }
    int Zerocoin_Block_FirstFraudulent() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nBlockFirstFraudulent; }
    int Zerocoin_Block_LastGoodCheckpoint() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nBlockLastGoodCheckpoint; }
    int Zerocoin_StartTime() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nZerocoinStartTime; }
    int Block_Enforce_Invalid() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nBlockEnforceInvalidUTXO; }
    int Zerocoin_Block_V2_Start() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nBlockZerocoinV2; }
    CAmount InvalidAmountFiltered() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nInvalidAmountFiltered; };

    CAmount BlockReward() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nBlockReward; }
    CAmount BlockReward2() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nBlockReward2; }
    CAmount MaxSupply() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMaxSupply; }
    CAmount BlocksPerYear() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nBlocksPerYear; }
    CAmount MasternodeTolerance() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMasternodeTolerance; }

protected:
    CChainParams() {}

    uint256 hashGenesisBlock;
    MessageStartChars pchMessageStart;
    //! Raw pub key bytes for the broadcast alert signing key.
    std::vector<unsigned char> vAlertPubKey;
    int nDefaultPort;
    uint256 bnProofOfWorkLimit;
    int nMaxReorganizationDepth;
    int nSubsidyHalvingInterval;
    int nEnforceBlockUpgradeMajority;
    int nRejectBlockOutdatedMajority;
    int nToCheckBlockUpgradeMajority;
    int64_t nTargetTimespan;
    int64_t nTargetSpacing;
    int nLastPOWBlock;
    int nMasternodeCountDrift;
    int nMaturity;
    int nModifierUpdateBlock;
    CAmount nMaxMoneyOut;
    int nMinerThreads;
    std::vector<CDNSSeedData> vSeeds;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
    CBaseChainParams::Network networkID;
    std::string strNetworkID;
    CBlock genesis;
    std::vector<CAddress> vFixedSeeds;
    bool fMiningRequiresPeers;
    bool fAllowMinDifficultyBlocks;
    bool fDefaultConsistencyChecks;
    bool fRequireStandard;
    bool fMineBlocksOnDemand;
    bool fSkipProofOfWorkCheck;
    bool fTestnetToBeDeprecatedFieldRPC;
    bool fHeadersFirstSyncingActive;
    bool fZeroCoinEnabled;
    int nPoolMaxTransactions;
    std::string strSporkKey;
    std::string strSporkKeyOld;
    int64_t nEnforceNewSporkKey;
    int64_t nRejectOldSporkKey;
    std::string strObfuscationPoolDummyAddress;
    int64_t nStartMasternodePayments;
    std::string zerocoinModulus;
    int64_t nSwapAmount;
    int64_t nSwapPoWBlocks;
    int64_t nSwapCoinbaseValue;
    int nMaxZerocoinSpendsPerTransaction;
    CAmount nMinZerocoinMintFee;
    CAmount nInvalidAmountFiltered;
    int nMintRequiredConfirmations;
    int nRequiredAccumulation;
    int nDefaultSecurityLevel;
    int nZerocoinHeaderVersion;
    int64_t nBudget_Fee_Confirmations;
    int nZerocoinStartHeight;
    int nZerocoinStartTime;
    int nZerocoinRequiredStakeDepth;

    int nBlockEnforceSerialRange;
    int nBlockRecalculateAccumulators;
    int nBlockFirstFraudulent;
    int nBlockLastGoodCheckpoint;
    int nBlockEnforceInvalidUTXO;
    int nBlockZerocoinV2;
    int nMasternodeTolerance;

    CAmount nBlockReward;
    CAmount nBlockReward2;
    CAmount nMaxSupply;

    int nBlocksPerYear;
};

/**
 * Modifiable parameters interface is used by test cases to adapt the parameters in order
 * to test specific features more easily. Test cases should always restore the previous
 * values after finalization.
 */

class CModifiableParams
{
public:
    //! Published setters to allow changing values in unit test cases
    virtual void setSubsidyHalvingInterval(int anSubsidyHalvingInterval) = 0;
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority) = 0;
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority) = 0;
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority) = 0;
    virtual void setDefaultConsistencyChecks(bool aDefaultConsistencyChecks) = 0;
    virtual void setAllowMinDifficultyBlocks(bool aAllowMinDifficultyBlocks) = 0;
    virtual void setSkipProofOfWorkCheck(bool aSkipProofOfWorkCheck) = 0;
};


/**
 * Return the currently selected parameters. This won't change after app startup
 * outside of the unit tests.
 */
const CChainParams& Params();

/** Return parameters for the given network. */
CChainParams& Params(CBaseChainParams::Network network);

/** Get modifiable network parameters (UNITTEST only) */
CModifiableParams* ModifiableParams();

/** Sets the params returned by Params() to those for the given network. */
void SelectParams(CBaseChainParams::Network network);

/**
 * Looks for -regtest or -testnet and then calls SelectParams as appropriate.
 * Returns false if an invalid combination is given.
 */
bool SelectParamsFromCommandLine();

#endif // BITCOIN_CHAINPARAMS_H
