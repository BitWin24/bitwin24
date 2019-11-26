#include "trace-log.h" //++++++++++++++++++
// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2017 The PIVX developers
// Copyright (c) 2018 The MAC developers
// Copyright (c) 2019 The BITWIN24 developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "keystore.h"

#include "crypter.h"
#include "key.h"
#include "script/script.h"
#include "script/standard.h"
#include "util.h"

#include <boost/foreach.hpp>

bool CKeyStore::GetPubKey(const CKeyID& address, CPubKey& vchPubKeyOut) const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    CKey key;
    if (!GetKey(address, key))
        return false;
    vchPubKeyOut = key.GetPubKey();
    return true;
}

bool CKeyStore::AddKey(const CKey& key)
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    return AddKeyPubKey(key, key.GetPubKey());
}

bool CBasicKeyStore::AddKeyPubKey(const CKey& key, const CPubKey& pubkey)
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    mapKeys[pubkey.GetID()] = key;
    return true;
}

bool CBasicKeyStore::AddCScript(const CScript& redeemScript)
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    if (redeemScript.size() > MAX_SCRIPT_ELEMENT_SIZE)
        return error("CBasicKeyStore::AddCScript() : redeemScripts > %i bytes are invalid", MAX_SCRIPT_ELEMENT_SIZE);

    LOCK(cs_KeyStore);
    mapScripts[CScriptID(redeemScript)] = redeemScript;
    return true;
}

bool CBasicKeyStore::HaveCScript(const CScriptID& hash) const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    return mapScripts.count(hash) > 0;
}

bool CBasicKeyStore::GetCScript(const CScriptID& hash, CScript& redeemScriptOut) const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    ScriptMap::const_iterator mi = mapScripts.find(hash);
    if (mi != mapScripts.end()) {
        redeemScriptOut = (*mi).second;
        return true;
    }
    return false;
}

bool CBasicKeyStore::AddWatchOnly(const CScript& dest)
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    setWatchOnly.insert(dest);
    return true;
}

bool CBasicKeyStore::RemoveWatchOnly(const CScript& dest)
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    setWatchOnly.erase(dest);
    return true;
}

bool CBasicKeyStore::HaveWatchOnly(const CScript& dest) const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    return setWatchOnly.count(dest) > 0;
}

bool CBasicKeyStore::HaveWatchOnly() const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    return (!setWatchOnly.empty());
}

bool CBasicKeyStore::AddMultiSig(const CScript& dest)
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    setMultiSig.insert(dest);
    return true;
}

bool CBasicKeyStore::RemoveMultiSig(const CScript& dest)
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    setMultiSig.erase(dest);
    return true;
}

bool CBasicKeyStore::HaveMultiSig(const CScript& dest) const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    return setMultiSig.count(dest) > 0;
}

bool CBasicKeyStore::HaveMultiSig() const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    LOCK(cs_KeyStore);
    return (!setMultiSig.empty());
}

bool CBasicKeyStore::HaveKey(const CKeyID& address) const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    bool result;
    {

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
        LOCK(cs_KeyStore);
        result = (mapKeys.count(address) > 0);
    }
    return result;
}

void CBasicKeyStore::GetKeys(std::set<CKeyID>& setAddress) const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    setAddress.clear();
    {

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
        LOCK(cs_KeyStore);
        KeyMap::const_iterator mi = mapKeys.begin();
        while (mi != mapKeys.end()) {
            setAddress.insert((*mi).first);
            mi++;
        }
    }
}

bool CBasicKeyStore::GetKey(const CKeyID& address, CKey& keyOut) const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    {

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
        LOCK(cs_KeyStore);
        KeyMap::const_iterator mi = mapKeys.find(address);
        if (mi != mapKeys.end()) {
            keyOut = mi->second;
            return true;
        }
    }
    return false;
}