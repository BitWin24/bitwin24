#include "/home/s/workspace/BitWin24/src/trace-log.h" //++++++++++++++++++
// Copyright (c) 2011-2013 The Bitcoin developers
// Copyright (c) 2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletmodeltransaction.h"

#include "wallet.h"

WalletModelTransaction::WalletModelTransaction(const QList<SendCoinsRecipient>& recipients) : recipients(recipients),
                                                                                              walletTransaction(0),
                                                                                              keyChange(0),
                                                                                              fee(0)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    walletTransaction = new CWalletTx();
}

WalletModelTransaction::~WalletModelTransaction()
{
    delete keyChange;
    delete walletTransaction;
}

QList<SendCoinsRecipient> WalletModelTransaction::getRecipients()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return recipients;
}

CWalletTx* WalletModelTransaction::getTransaction()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return walletTransaction;
}

unsigned int WalletModelTransaction::getTransactionSize()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return (!walletTransaction ? 0 : (::GetSerializeSize(*(CTransaction*)walletTransaction, SER_NETWORK, PROTOCOL_VERSION)));
}

CAmount WalletModelTransaction::getTransactionFee()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return fee;
}

void WalletModelTransaction::setTransactionFee(const CAmount& newFee)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    fee = newFee;
}

CAmount WalletModelTransaction::getTotalTransactionAmount()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    CAmount totalTransactionAmount = 0;
    foreach (const SendCoinsRecipient& rcp, recipients) {
        totalTransactionAmount += rcp.amount;
    }
    return totalTransactionAmount;
}

void WalletModelTransaction::newPossibleKeyChange(CWallet* wallet)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    keyChange = new CReserveKey(wallet);
}

CReserveKey* WalletModelTransaction::getPossibleKeyChange()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return keyChange;
}
