#pragma once

#include "leveldbwrapper.h"
#include "base58.h"

#include <unordered_set>
#include <string>

/*
 * Contains addresses excluded from staking
 */
class StakingAccountsDb
{
public:
    static StakingAccountsDb& instance();

public:
    StakingAccountsDb();

    bool exist( const CBitcoinAddress& address ) const;
    bool add( const CBitcoinAddress& address );
    bool remove( const CBitcoinAddress& address );
    std::set< CBitcoinAddress > get() const;

private:
    CLevelDBWrapper _db;
};