#include "/home/s/workspace/BitWin24/src/trace-log.h" //++++++++++++++++++
// Copyright (c) 2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "zmqabstractnotifier.h"
#include "util.h"


CZMQAbstractNotifier::~CZMQAbstractNotifier()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    assert(!psocket);
}

bool CZMQAbstractNotifier::NotifyBlock(const CBlockIndex * /*CBlockIndex*/)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return true;
}

bool CZMQAbstractNotifier::NotifyTransaction(const CTransaction &/*transaction*/)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return true;
}

bool CZMQAbstractNotifier::NotifyTransactionLock(const CTransaction &/*transaction*/)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return true;
}
