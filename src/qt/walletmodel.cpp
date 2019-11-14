#include "/home/s/workspace/BitWin24/src/trace-log.h" //++++++++++++++++++
// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletmodel.h"

#include "addresstablemodel.h"
#include "guiconstants.h"
#include "recentrequeststablemodel.h"
#include "transactiontablemodel.h"

#include "base58.h"
#include "db.h"
#include "keystore.h"
#include "main.h"
#include "spork.h"
#include "sync.h"
#include "ui_interface.h"
#include "wallet.h"
#include "walletdb.h" // for BackupWallet
#include <stdint.h>

#include <QDebug>
#include <QSet>
#include <QTimer>

using namespace std;

WalletModel::WalletModel(CWallet* wallet, OptionsModel* optionsModel, QObject* parent) : QObject(parent), wallet(wallet), optionsModel(optionsModel), addressTableModel(0),
                                                                                         transactionTableModel(0),
                                                                                         recentRequestsTableModel(0),
                                                                                         cachedBalance(0), cachedUnconfirmedBalance(0), cachedImmatureBalance(0),
                                                                                         cachedZerocoinBalance(0), cachedUnconfirmedZerocoinBalance(0), cachedImmatureZerocoinBalance(0),
                                                                                         cachedEarnings(0), cachedMasternodeEarnings(0), cachedStakeEarnings(0),
                                                                                         cachedEncryptionStatus(Unencrypted),
                                                                                         cachedNumBlocks(0)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    fHaveWatchOnly = wallet->HaveWatchOnly();
    fHaveMultiSig = wallet->HaveMultiSig();
    fForceCheckBalanceChanged = false;

    addressTableModel = new AddressTableModel(wallet, this);
    transactionTableModel = new TransactionTableModel(wallet, this);
    recentRequestsTableModel = new RecentRequestsTableModel(wallet, this);

    // This timer will be fired repeatedly to update the balance
    pollTimer = new QTimer(this);
    connect(pollTimer, SIGNAL(timeout()), this, SLOT(pollBalanceChanged()));
    pollTimer->start(MODEL_UPDATE_DELAY);

    subscribeToCoreSignals();
}

WalletModel::~WalletModel()
{
    unsubscribeFromCoreSignals();
}

CAmount WalletModel::getBalance(const CCoinControl* coinControl) const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    if (coinControl) {
        CAmount nBalance = 0;
        std::vector<COutput> vCoins;
        wallet->AvailableCoins(vCoins, true, coinControl);
        BOOST_FOREACH (const COutput& out, vCoins)
            if (out.fSpendable)
                nBalance += out.tx->vout[out.i].nValue;

        return nBalance;
    }

    return wallet->GetBalance();
}

CAmount WalletModel::getStakeEarnings() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetEarnings(false) - wallet->GetEarnings(true);
}

CAmount WalletModel::getMasternodeEarnings() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetEarnings(true);
}

CAmount WalletModel::getEarnings() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetEarnings(false);
}

CAmount WalletModel::getUnconfirmedBalance() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetUnconfirmedBalance();
}

CAmount WalletModel::getImmatureBalance() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetImmatureBalance();
}

CAmount WalletModel::getLockedBalance() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetLockedCoins();
}

CAmount WalletModel::getZerocoinBalance() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetZerocoinBalance(false);
}

CAmount WalletModel::getUnconfirmedZerocoinBalance() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetUnconfirmedZerocoinBalance();
}

CAmount WalletModel::getImmatureZerocoinBalance() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetImmatureZerocoinBalance();
}


bool WalletModel::haveWatchOnly() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return fHaveWatchOnly;
}

CAmount WalletModel::getWatchBalance() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetWatchOnlyBalance();
}

CAmount WalletModel::getWatchUnconfirmedBalance() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetUnconfirmedWatchOnlyBalance();
}

CAmount WalletModel::getWatchImmatureBalance() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetImmatureWatchOnlyBalance();
}

void WalletModel::updateStatus()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    EncryptionStatus newEncryptionStatus = getEncryptionStatus();

    if (cachedEncryptionStatus != newEncryptionStatus)
        emit encryptionStatusChanged(newEncryptionStatus);
}

void WalletModel::pollBalanceChanged()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    // Get required locks upfront. This avoids the GUI from getting stuck on
    // periodical polls if the core is holding the locks for a longer time -
    // for example, during a wallet rescan.
    TRY_LOCK(cs_main, lockMain);
    if (!lockMain)
        return;
    TRY_LOCK(wallet->cs_wallet, lockWallet);
    if (!lockWallet)
        return;

    if (fForceCheckBalanceChanged || chainActive.Height() != cachedNumBlocks || nZeromintPercentage != cachedZeromintPercentage || cachedTxLocks != nCompleteTXLocks) {
        fForceCheckBalanceChanged = false;

        // Balance and number of transactions might have changed
        cachedNumBlocks = chainActive.Height();
        cachedZeromintPercentage = nZeromintPercentage;

        checkBalanceChanged();
        if (transactionTableModel) {
            transactionTableModel->updateConfirmations();
        }

        // Address in receive tab may have been used
        emit notifyReceiveAddressChanged();
    }
}

void WalletModel::emitBalanceChanged()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    // Force update of UI elements even when no values have changed
    emit balanceChanged(cachedBalance, cachedUnconfirmedBalance, cachedImmatureBalance, 
                        cachedZerocoinBalance, cachedUnconfirmedZerocoinBalance, cachedImmatureZerocoinBalance,
                        cachedWatchOnlyBalance, cachedWatchUnconfBalance, cachedWatchImmatureBalance,
                            cachedEarnings, cachedMasternodeEarnings, cachedStakeEarnings);
}

void WalletModel::checkBalanceChanged()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    TRY_LOCK(cs_main, lockMain);
    if (!lockMain) return;

    CAmount newBalance = getBalance();
    CAmount newUnconfirmedBalance = getUnconfirmedBalance();
    CAmount newImmatureBalance = getImmatureBalance();
    CAmount newZerocoinBalance = getZerocoinBalance();
    CAmount newUnconfirmedZerocoinBalance = getUnconfirmedZerocoinBalance();
    CAmount newImmatureZerocoinBalance = getImmatureZerocoinBalance();
    CAmount newEarnings = getEarnings();
    CAmount newMasternodeEarnings = getMasternodeEarnings();
    CAmount newStakeEarnings = getStakeEarnings();
    CAmount newWatchOnlyBalance = 0;
    CAmount newWatchUnconfBalance = 0;
    CAmount newWatchImmatureBalance = 0;
    if (haveWatchOnly()) {
        newWatchOnlyBalance = getWatchBalance();
        newWatchUnconfBalance = getWatchUnconfirmedBalance();
        newWatchImmatureBalance = getWatchImmatureBalance();
    }

    if (cachedBalance != newBalance || cachedUnconfirmedBalance != newUnconfirmedBalance || cachedImmatureBalance != newImmatureBalance ||
        cachedZerocoinBalance != newZerocoinBalance || cachedUnconfirmedZerocoinBalance != newUnconfirmedZerocoinBalance || cachedImmatureZerocoinBalance != newImmatureZerocoinBalance ||
        cachedWatchOnlyBalance != newWatchOnlyBalance || cachedWatchUnconfBalance != newWatchUnconfBalance || cachedWatchImmatureBalance != newWatchImmatureBalance ||
        cachedTxLocks != nCompleteTXLocks 
        || cachedEarnings != newEarnings || cachedMasternodeEarnings != newMasternodeEarnings || cachedStakeEarnings != newStakeEarnings) {
        cachedBalance = newBalance;
        cachedUnconfirmedBalance = newUnconfirmedBalance;
        cachedImmatureBalance = newImmatureBalance;
        cachedZerocoinBalance = newZerocoinBalance;
        cachedUnconfirmedZerocoinBalance = newUnconfirmedZerocoinBalance;
        cachedImmatureZerocoinBalance = newImmatureZerocoinBalance;
        cachedEarnings = newEarnings;
        cachedMasternodeEarnings = newMasternodeEarnings;
        cachedStakeEarnings = newStakeEarnings;
        cachedTxLocks = nCompleteTXLocks;
        cachedWatchOnlyBalance = newWatchOnlyBalance;
        cachedWatchUnconfBalance = newWatchUnconfBalance;
        cachedWatchImmatureBalance = newWatchImmatureBalance;
        emit balanceChanged(newBalance, newUnconfirmedBalance, newImmatureBalance, 
                            newZerocoinBalance, newUnconfirmedZerocoinBalance, newImmatureZerocoinBalance,
                            newWatchOnlyBalance, newWatchUnconfBalance, newWatchImmatureBalance,
                            newEarnings, newMasternodeEarnings, newStakeEarnings);
    }
}

void WalletModel::updateTransaction()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    // Balance and number of transactions might have changed
    fForceCheckBalanceChanged = true;
}

void WalletModel::updateAddressBook(const QString& address, const QString& label, bool isMine, const QString& purpose, int status)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    if (addressTableModel)
        addressTableModel->updateEntry(address, label, isMine, purpose, status);
}
void WalletModel::updateAddressBook(const QString &pubCoin, const QString &isUsed, int status)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    if(addressTableModel)
        addressTableModel->updateEntry(pubCoin, isUsed, status);
}


void WalletModel::updateWatchOnlyFlag(bool fHaveWatchonly)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    fHaveWatchOnly = fHaveWatchonly;
    emit notifyWatchonlyChanged(fHaveWatchonly);
}

void WalletModel::updateMultiSigFlag(bool fHaveMultiSig)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    this->fHaveMultiSig = fHaveMultiSig;
    emit notifyMultiSigChanged(fHaveMultiSig);
}

bool WalletModel::validateAddress(const QString& address)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    CBitcoinAddress addressParsed(address.toStdString());
    return addressParsed.IsValid();
}

void WalletModel::updateAddressBookLabels(const CTxDestination& dest, const string& strName, const string& strPurpose)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    LOCK(wallet->cs_wallet);

    std::map<CTxDestination, CAddressBookData>::iterator mi = wallet->mapAddressBook.find(dest);

    // Check if we have a new address or an updated label
    if (mi == wallet->mapAddressBook.end()) {
        wallet->SetAddressBook(dest, strName, strPurpose);
    } else if (mi->second.name != strName) {
        wallet->SetAddressBook(dest, strName, ""); // "" means don't change purpose
    }
}

WalletModel::SendCoinsReturn WalletModel::prepareTransaction(WalletModelTransaction& transaction, const CCoinControl* coinControl)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    CAmount total = 0;
    QList<SendCoinsRecipient> recipients = transaction.getRecipients();
    std::vector<std::pair<CScript, CAmount> > vecSend;

    if (recipients.empty()) {
        return OK;
    }

    if (isAnonymizeOnlyUnlocked()) {
        return AnonymizeOnlyUnlocked;
    }

    QSet<QString> setAddress; // Used to detect duplicates
    int nAddresses = 0;

    // Pre-check input data for validity
    foreach (const SendCoinsRecipient& rcp, recipients) {
        if (rcp.paymentRequest.IsInitialized()) { // PaymentRequest...
            CAmount subtotal = 0;
            const payments::PaymentDetails& details = rcp.paymentRequest.getDetails();
            for (int i = 0; i < details.outputs_size(); i++) {
                const payments::Output& out = details.outputs(i);
                if (out.amount() <= 0) continue;
                subtotal += out.amount();
                const unsigned char* scriptStr = (const unsigned char*)out.script().data();
                CScript scriptPubKey(scriptStr, scriptStr + out.script().size());
                vecSend.push_back(std::pair<CScript, CAmount>(scriptPubKey, out.amount()));
            }
            if (subtotal <= 0) {
                return InvalidAmount;
            }
            total += subtotal;
        } else { // User-entered bitwin24 address / amount:
            if (!validateAddress(rcp.address)) {
                return InvalidAddress;
            }
            if (rcp.amount <= 0) {
                return InvalidAmount;
            }
            setAddress.insert(rcp.address);
            ++nAddresses;

            CScript scriptPubKey = GetScriptForDestination(CBitcoinAddress(rcp.address.toStdString()).Get());
            vecSend.push_back(std::pair<CScript, CAmount>(scriptPubKey, rcp.amount));

            total += rcp.amount;
        }
    }
    if (setAddress.size() != nAddresses) {
        return DuplicateAddress;
    }

    CAmount nBalance = getBalance(coinControl);

    if (total > nBalance) {
        return AmountExceedsBalance;
    }

    {
        LOCK2(cs_main, wallet->cs_wallet);

        transaction.newPossibleKeyChange(wallet);
        CAmount nFeeRequired = 0;
        std::string strFailReason;

        CWalletTx* newTx = transaction.getTransaction();
        CReserveKey* keyChange = transaction.getPossibleKeyChange();


        if (recipients[0].useSwiftTX && total > GetSporkValue(SPORK_5_MAX_VALUE) * COIN) {
            emit message(tr("Send Coins"), tr("SwiftX doesn't support sending values that high yet. Transactions are currently limited to %1 BITWIN24.").arg(GetSporkValue(SPORK_5_MAX_VALUE)),
                CClientUIInterface::MSG_ERROR);
            return TransactionCreationFailed;
        }

        bool fCreated = wallet->CreateTransaction(vecSend, *newTx, *keyChange, nFeeRequired, strFailReason, coinControl, recipients[0].inputType, recipients[0].useSwiftTX);
        transaction.setTransactionFee(nFeeRequired);

        if (recipients[0].useSwiftTX && newTx->GetValueOut() > GetSporkValue(SPORK_5_MAX_VALUE) * COIN) {
            emit message(tr("Send Coins"), tr("SwiftX doesn't support sending values that high yet. Transactions are currently limited to %1 BITWIN24.").arg(GetSporkValue(SPORK_5_MAX_VALUE)),
                CClientUIInterface::MSG_ERROR);
            return TransactionCreationFailed;
        }

        if (!fCreated) {
            if ((total + nFeeRequired) > nBalance) {
                return SendCoinsReturn(AmountWithFeeExceedsBalance);
            }
            emit message(tr("Send Coins"), QString::fromStdString(strFailReason),
                CClientUIInterface::MSG_ERROR);
            return TransactionCreationFailed;
        }

        // reject insane fee
        if (nFeeRequired > ::minRelayTxFee.GetFee(transaction.getTransactionSize()) * 10000)
            return InsaneFee;
    }

    return SendCoinsReturn(OK);
}

WalletModel::SendCoinsReturn WalletModel::sendCoins(WalletModelTransaction& transaction)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    QByteArray transaction_array; /* store serialized transaction */

    if (isAnonymizeOnlyUnlocked()) {
        return AnonymizeOnlyUnlocked;
    }

    {
        LOCK2(cs_main, wallet->cs_wallet);
        CWalletTx* newTx = transaction.getTransaction();
        QList<SendCoinsRecipient> recipients = transaction.getRecipients();

        // Store PaymentRequests in wtx.vOrderForm in wallet.
        foreach (const SendCoinsRecipient& rcp, recipients) {
            if (rcp.paymentRequest.IsInitialized()) {
                std::string key("PaymentRequest");
                std::string value;
                rcp.paymentRequest.SerializeToString(&value);
                newTx->vOrderForm.push_back(make_pair(key, value));
            } else if (!rcp.message.isEmpty()) // Message from normal bitwin24:URI (bitwin24:XyZ...?message=example)
            {
                newTx->vOrderForm.push_back(make_pair("Message", rcp.message.toStdString()));
            }
        }

        CReserveKey* keyChange = transaction.getPossibleKeyChange();

        transaction.getRecipients();

        if (!wallet->CommitTransaction(*newTx, *keyChange, (recipients[0].useSwiftTX) ? "ix" : "tx"))
            return TransactionCommitFailed;

        CTransaction* t = (CTransaction*)newTx;
        CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
        ssTx << *t;
        transaction_array.append(&(ssTx[0]), ssTx.size());
    }

    // Add addresses / update labels that we've sent to to the address book,
    // and emit coinsSent signal for each recipient
    foreach (const SendCoinsRecipient& rcp, transaction.getRecipients()) {
        // Don't touch the address book when we have a payment request
        if (!rcp.paymentRequest.IsInitialized()) {

            std::string strAddress = rcp.address.toStdString();
            CTxDestination dest = CBitcoinAddress(strAddress).Get();
            std::string strLabel = rcp.label.toStdString();

            updateAddressBookLabels(dest, strLabel, "send");
        }
        emit coinsSent(wallet, rcp, transaction_array);
    }
    checkBalanceChanged(); // update balance immediately, otherwise there could be a short noticeable delay until pollBalanceChanged hits

    return SendCoinsReturn(OK);
}

OptionsModel* WalletModel::getOptionsModel()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return optionsModel;
}

AddressTableModel* WalletModel::getAddressTableModel()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return addressTableModel;
}

TransactionTableModel* WalletModel::getTransactionTableModel()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return transactionTableModel;
}

RecentRequestsTableModel* WalletModel::getRecentRequestsTableModel()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return recentRequestsTableModel;
}

WalletModel::EncryptionStatus WalletModel::getEncryptionStatus() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    if (!wallet->IsCrypted()) {
        return Unencrypted;
    } else if (wallet->fWalletUnlockAnonymizeOnly) {
        return UnlockedForAnonymizationOnly;
    } else if (wallet->IsLocked()) {
        return Locked;
    } else {
        return Unlocked;
    }

}

bool WalletModel::setWalletEncrypted(bool encrypted, const SecureString& passphrase)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    if (encrypted) {
        // Encrypt
        return wallet->EncryptWallet(passphrase);
    } else {
        // Decrypt -- TODO; not supported yet
        return false;
    }
}

bool WalletModel::setWalletLocked(bool locked, const SecureString& passPhrase, bool anonymizeOnly)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    if (locked) {
        // Lock
        wallet->fWalletUnlockAnonymizeOnly = false;
        return wallet->Lock();
    } else {
        // Unlock
        return wallet->Unlock(passPhrase, anonymizeOnly);
    }
}

bool WalletModel::isAnonymizeOnlyUnlocked()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->fWalletUnlockAnonymizeOnly;
}

bool WalletModel::changePassphrase(const SecureString& oldPass, const SecureString& newPass)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    bool retval;
    {
        LOCK(wallet->cs_wallet);
        wallet->Lock(); // Make sure wallet is locked before attempting pass change
        retval = wallet->ChangeWalletPassphrase(oldPass, newPass);
    }
    return retval;
}

bool WalletModel::backupWallet(const QString& filename)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    //attempt regular backup
    if(!BackupWallet(*wallet, filename.toLocal8Bit().data())) {
        return error("ERROR: Failed to backup wallet!");
    }

    return true;
}


// Handlers for core signals
static void NotifyKeyStoreStatusChanged(WalletModel* walletmodel, CCryptoKeyStore* wallet)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    qDebug() << "NotifyKeyStoreStatusChanged";
    QMetaObject::invokeMethod(walletmodel, "updateStatus", Qt::QueuedConnection);
}

static void NotifyAddressBookChanged(WalletModel* walletmodel, CWallet* wallet, const CTxDestination& address, const std::string& label, bool isMine, const std::string& purpose, ChangeType status)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    QString strAddress = QString::fromStdString(CBitcoinAddress(address).ToString());
    QString strLabel = QString::fromStdString(label);
    QString strPurpose = QString::fromStdString(purpose);

    qDebug() << "NotifyAddressBookChanged : " + strAddress + " " + strLabel + " isMine=" + QString::number(isMine) + " purpose=" + strPurpose + " status=" + QString::number(status);
    QMetaObject::invokeMethod(walletmodel, "updateAddressBook", Qt::QueuedConnection,
        Q_ARG(QString, strAddress),
        Q_ARG(QString, strLabel),
        Q_ARG(bool, isMine),
        Q_ARG(QString, strPurpose),
        Q_ARG(int, status));
}

// queue notifications to show a non freezing progress dialog e.g. for rescan
static bool fQueueNotifications = false;
static std::vector<std::pair<uint256, ChangeType> > vQueueNotifications;
static void NotifyTransactionChanged(WalletModel* walletmodel, CWallet* wallet, const uint256& hash, ChangeType status)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    if (fQueueNotifications) {
        vQueueNotifications.push_back(make_pair(hash, status));
        return;
    }

    QString strHash = QString::fromStdString(hash.GetHex());

    qDebug() << "NotifyTransactionChanged : " + strHash + " status= " + QString::number(status);
    QMetaObject::invokeMethod(walletmodel, "updateTransaction", Qt::QueuedConnection /*,
                              Q_ARG(QString, strHash),
                              Q_ARG(int, status)*/);
}

static void ShowProgress(WalletModel* walletmodel, const std::string& title, int nProgress)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    // emits signal "showProgress"
    QMetaObject::invokeMethod(walletmodel, "showProgress", Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(title)),
        Q_ARG(int, nProgress));
}

static void NotifyWatchonlyChanged(WalletModel* walletmodel, bool fHaveWatchonly)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    QMetaObject::invokeMethod(walletmodel, "updateWatchOnlyFlag", Qt::QueuedConnection,
        Q_ARG(bool, fHaveWatchonly));
}

static void NotifyMultiSigChanged(WalletModel* walletmodel, bool fHaveMultiSig)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    QMetaObject::invokeMethod(walletmodel, "updateMultiSigFlag", Qt::QueuedConnection,
                              Q_ARG(bool, fHaveMultiSig));
}

static void NotifyZerocoinChanged(WalletModel* walletmodel, CWallet* wallet, const std::string& hexString,
                                  const std::string& isUsed, ChangeType status)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    QString HexStr = QString::fromStdString(hexString);
    QString isUsedStr = QString::fromStdString(isUsed);
    qDebug() << "NotifyZerocoinChanged : " + HexStr + " " + isUsedStr + " status= " + QString::number(status);
    QMetaObject::invokeMethod(walletmodel, "updateAddressBook", Qt::QueuedConnection,
                              Q_ARG(QString, HexStr),
                              Q_ARG(QString, isUsedStr),
                              Q_ARG(int, status));
}

static void NotifyzBWIReset(WalletModel* walletmodel)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    qDebug() << "NotifyzBWIReset";
    QMetaObject::invokeMethod(walletmodel, "checkBalanceChanged", Qt::QueuedConnection);
}

static void NotifyWalletBacked(WalletModel* model, const bool& fSuccess, const string& filename)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    string message;
    string title = "Backup ";
    CClientUIInterface::MessageBoxFlags method;

    if (fSuccess) {
        message = "The wallet data was successfully saved to ";
        title += "Successful: ";
        method = CClientUIInterface::MessageBoxFlags::MSG_INFORMATION;
    } else {
        message = "There was an error trying to save the wallet data to ";
        title += "Failed: ";
        method = CClientUIInterface::MessageBoxFlags::MSG_ERROR;
    }

    message += _(filename.data());

    QMetaObject::invokeMethod(model, "message", Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromStdString(title)),
                              Q_ARG(QString, QString::fromStdString(message)),
                              Q_ARG(unsigned int, (unsigned int)method));
}

void WalletModel::subscribeToCoreSignals()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    // Connect signals to wallet
    wallet->NotifyStatusChanged.connect(boost::bind(&NotifyKeyStoreStatusChanged, this, _1));
    wallet->NotifyAddressBookChanged.connect(boost::bind(NotifyAddressBookChanged, this, _1, _2, _3, _4, _5, _6));
    wallet->NotifyTransactionChanged.connect(boost::bind(NotifyTransactionChanged, this, _1, _2, _3));
    wallet->ShowProgress.connect(boost::bind(ShowProgress, this, _1, _2));
    wallet->NotifyWatchonlyChanged.connect(boost::bind(NotifyWatchonlyChanged, this, _1));
    wallet->NotifyMultiSigChanged.connect(boost::bind(NotifyMultiSigChanged, this, _1));
    wallet->NotifyZerocoinChanged.connect(boost::bind(NotifyZerocoinChanged, this, _1, _2, _3, _4));
    wallet->NotifyzBWIReset.connect(boost::bind(NotifyzBWIReset, this));
    wallet->NotifyWalletBacked.connect(boost::bind(NotifyWalletBacked, this, _1, _2));
}

void WalletModel::unsubscribeFromCoreSignals()
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    // Disconnect signals from wallet
    wallet->NotifyStatusChanged.disconnect(boost::bind(&NotifyKeyStoreStatusChanged, this, _1));
    wallet->NotifyAddressBookChanged.disconnect(boost::bind(NotifyAddressBookChanged, this, _1, _2, _3, _4, _5, _6));
    wallet->NotifyTransactionChanged.disconnect(boost::bind(NotifyTransactionChanged, this, _1, _2, _3));
    wallet->ShowProgress.disconnect(boost::bind(ShowProgress, this, _1, _2));
    wallet->NotifyWatchonlyChanged.disconnect(boost::bind(NotifyWatchonlyChanged, this, _1));
    wallet->NotifyMultiSigChanged.disconnect(boost::bind(NotifyMultiSigChanged, this, _1));
    wallet->NotifyZerocoinChanged.disconnect(boost::bind(NotifyZerocoinChanged, this, _1, _2, _3, _4));
    wallet->NotifyzBWIReset.disconnect(boost::bind(NotifyzBWIReset, this));
    wallet->NotifyWalletBacked.disconnect(boost::bind(NotifyWalletBacked, this, _1, _2));
}

// WalletModel::UnlockContext implementation
WalletModel::UnlockContext WalletModel::requestUnlock(AskPassphraseDialog::Context context, bool relock)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    bool was_locked = getEncryptionStatus() == Locked;

    if (!was_locked && isAnonymizeOnlyUnlocked()) {
        setWalletLocked(true);
        wallet->fWalletUnlockAnonymizeOnly = false;
        was_locked = getEncryptionStatus() == Locked;
    }

    if (was_locked) {
        // Request UI to unlock wallet
        emit requireUnlock(context);
    }
    // If wallet is still locked, unlock was failed or cancelled, mark context as invalid
    bool valid = getEncryptionStatus() != Locked;

    return UnlockContext(valid, relock);
    //    return UnlockContext(this, valid, was_locked && !isAnonymizeOnlyUnlocked());
}

WalletModel::UnlockContext::UnlockContext(bool valid, bool relock) : valid(valid), relock(relock)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

}

WalletModel::UnlockContext::~UnlockContext()
{
/*
    if (valid && relock) {
        wallet->setWalletLocked(true);
    }
*/
}

void WalletModel::UnlockContext::CopyFrom(const UnlockContext& rhs)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    // Transfer context; old object no longer relocks wallet
    *this = rhs;
    rhs.relock = false;
}

bool WalletModel::getPubKey(const CKeyID& address, CPubKey& vchPubKeyOut) const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->GetPubKey(address, vchPubKeyOut);
}

// returns a list of COutputs from COutPoints
void WalletModel::getOutputs(const std::vector<COutPoint>& vOutpoints, std::vector<COutput>& vOutputs)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    LOCK2(cs_main, wallet->cs_wallet);
    BOOST_FOREACH (const COutPoint& outpoint, vOutpoints) {
        if (!wallet->mapWallet.count(outpoint.hash)) continue;
        int nDepth = wallet->mapWallet[outpoint.hash].GetDepthInMainChain();
        if (nDepth < 0) continue;
        COutput out(&wallet->mapWallet[outpoint.hash], outpoint.n, nDepth, true);
        vOutputs.push_back(out);
    }
}

bool WalletModel::isSpent(const COutPoint& outpoint) const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    LOCK2(cs_main, wallet->cs_wallet);
    return wallet->IsSpent(outpoint.hash, outpoint.n);
}

// AvailableCoins + LockedCoins grouped by wallet address (put change in one group with wallet address)
void WalletModel::listCoins(std::map<QString, std::vector<COutput> >& mapCoins) const
{
    std::vector<COutput> vCoins;
    wallet->AvailableCoins(vCoins);

    LOCK2(cs_main, wallet->cs_wallet); // ListLockedCoins, mapWallet
    std::vector<COutPoint> vLockedCoins;
    wallet->ListLockedCoins(vLockedCoins);

    // add locked coins
    BOOST_FOREACH (const COutPoint& outpoint, vLockedCoins) {
        if (!wallet->mapWallet.count(outpoint.hash)) continue;
        int nDepth = wallet->mapWallet[outpoint.hash].GetDepthInMainChain();
        if (nDepth < 0) continue;
        COutput out(&wallet->mapWallet[outpoint.hash], outpoint.n, nDepth, true);
        if (outpoint.n < out.tx->vout.size() && wallet->IsMine(out.tx->vout[outpoint.n]) == ISMINE_SPENDABLE)
            vCoins.push_back(out);
    }

    BOOST_FOREACH (const COutput& out, vCoins) {
        COutput cout = out;

        while (wallet->IsChange(cout.tx->vout[cout.i]) && cout.tx->vin.size() > 0 && wallet->IsMine(cout.tx->vin[0])) {
            if (!wallet->mapWallet.count(cout.tx->vin[0].prevout.hash)) break;
            cout = COutput(&wallet->mapWallet[cout.tx->vin[0].prevout.hash], cout.tx->vin[0].prevout.n, 0, true);
        }

        CTxDestination address;
        if (!out.fSpendable || !ExtractDestination(cout.tx->vout[cout.i].scriptPubKey, address))
            continue;
        mapCoins[QString::fromStdString(CBitcoinAddress(address).ToString())].push_back(out);
    }
}

bool WalletModel::isLockedCoin(uint256 hash, unsigned int n) const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    LOCK2(cs_main, wallet->cs_wallet);
    return wallet->IsLockedCoin(hash, n);
}

void WalletModel::lockCoin(COutPoint& output)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    LOCK2(cs_main, wallet->cs_wallet);
    wallet->LockCoin(output);
}

void WalletModel::unlockCoin(COutPoint& output)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    LOCK2(cs_main, wallet->cs_wallet);
    wallet->UnlockCoin(output);
}

void WalletModel::listLockedCoins(std::vector<COutPoint>& vOutpts)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    LOCK2(cs_main, wallet->cs_wallet);
    wallet->ListLockedCoins(vOutpts);
}


void WalletModel::listZerocoinMints(std::set<CMintMeta>& setMints, bool fUnusedOnly, bool fMaturedOnly, bool fUpdateStatus)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    setMints.clear();
    setMints = pwalletMain->zbwiTracker->ListMints(fUnusedOnly, fMaturedOnly, fUpdateStatus);
}

void WalletModel::loadReceiveRequests(std::vector<std::string>& vReceiveRequests)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    LOCK(wallet->cs_wallet);
    BOOST_FOREACH (const PAIRTYPE(CTxDestination, CAddressBookData) & item, wallet->mapAddressBook)
        BOOST_FOREACH (const PAIRTYPE(std::string, std::string) & item2, item.second.destdata)
            if (item2.first.size() > 2 && item2.first.substr(0, 2) == "rr") // receive request
                vReceiveRequests.push_back(item2.second);
}

bool WalletModel::saveReceiveRequest(const std::string& sAddress, const int64_t nId, const std::string& sRequest)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    CTxDestination dest = CBitcoinAddress(sAddress).Get();

    std::stringstream ss;
    ss << nId;
    std::string key = "rr" + ss.str(); // "rr" prefix = "receive request" in destdata

    LOCK(wallet->cs_wallet);
    if (sRequest.empty())
        return wallet->EraseDestData(dest, key);
    else
        return wallet->AddDestData(dest, key, sRequest);
}

bool WalletModel::isMine(CBitcoinAddress address)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return IsMine(*wallet, address.Get());
}

bool WalletModel::isUsed(CBitcoinAddress address)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return wallet->IsUsed(address);
}
