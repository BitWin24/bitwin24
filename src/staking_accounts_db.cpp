#include "staking_accounts_db.h"

#include "util.h"

#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

StakingAccountsDb& StakingAccountsDb::instance()
{
    static StakingAccountsDb _instance;

    return _instance;
}

StakingAccountsDb::StakingAccountsDb():
    _db( GetDataDir() / "staking", 1 << 23, false)
{
}

bool StakingAccountsDb::exist( const CBitcoinAddress& address ) const
{
    return _db.Exists(Hash(address.ToString()));
}

bool StakingAccountsDb::add( const CBitcoinAddress& address )
{
    return _db.Write( Hash(address.ToString()), address.ToString(), true );
}

bool StakingAccountsDb::remove( const CBitcoinAddress& address )
{
    return _db.Erase( Hash(address.ToString()) );
}

std::set< CBitcoinAddress > StakingAccountsDb::get() const
{
    // It seems that there are no "const iterators" for LevelDB.
    // Since we only need read operations on it, use a const-cast to get around that restriction.
    boost::scoped_ptr<leveldb::Iterator> pcursor(const_cast<CLevelDBWrapper*>(&_db)->NewIterator());
    pcursor->SeekToFirst();

    std::set< CBitcoinAddress > stakingAccounts;
    while (pcursor->Valid()) {
        // WriteBatch stores data in format [VerPerfix + Key, Value]
        std::string address;
        CDataStream ssValue(pcursor->value().data(), pcursor->value().data() + pcursor->value().size(), SER_DISK, CLIENT_VERSION);
        ssValue >> address;

        stakingAccounts.insert( address );
        pcursor->Next();
    }

    return stakingAccounts;
}