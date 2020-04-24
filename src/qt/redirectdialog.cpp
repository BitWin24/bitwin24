// Copyright (c) 2017-2018 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "redirectdialog.h"
#include "ui_redirectdialog.h"

#include "addressbookpage.h"
#include "addresstablemodel.h"
#include "base58.h"
#include "init.h"
#include "walletmodel.h"

#include <QStyle>

RedirectDialog::RedirectDialog(QWidget* parent) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
                                                    ui(new Ui::RedirectDialog),
                                                    model(nullptr)
{
    ui->setupUi(this);
}

RedirectDialog::~RedirectDialog()
{
    delete ui;
}

void RedirectDialog::setModel(WalletModel* model)
{
    this->model = model;
}

void RedirectDialog::setFromAddress(const QString& address)
{
    setAddress(address, ui->redirectFromAddressEdit);
}

void RedirectDialog::setToAddress(const QString& address)
{
    setAddress(address, ui->redirectToAddressEdit);
}

void RedirectDialog::setAddress(const QString& address, QLineEdit* addrEdit)
{
    addrEdit->setText(address);
    addrEdit->setFocus();
}

void RedirectDialog::on_fromAddressBookButton_clicked()
{
    if (model && model->getAddressTableModel()) {
        AddressBookPage dlg(AddressBookPage::ForSelection, AddressBookPage::SendingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
            setAddress(dlg.getReturnValue(), ui->redirectFromAddressEdit);

        // Update the label text box with the label in the addressbook
//        QString associatedLabel = model->getAddressTableModel()->labelForAddress(dlg.getReturnValue());
//        if (!associatedLabel.isEmpty())
//            ui->labelAddressLabelEdit->setText(associatedLabel);
//        else
//            ui->labelAddressLabelEdit->setText(tr("(no label)"));
    }
}

void RedirectDialog::on_toAddressBookButton_clicked()
{
    if (model && model->getAddressTableModel()) {
        AddressBookPage dlg(AddressBookPage::ForSelection, AddressBookPage::SendingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
            setAddress(dlg.getReturnValue(), ui->redirectToAddressEdit);

        // Update the label text box with the label in the addressbook
//        QString associatedLabel = model->getAddressTableModel()->labelForAddress(dlg.getReturnValue());
//        if (!associatedLabel.isEmpty())
//            ui->labelAddressLabelEdit->setText(associatedLabel);
//        else
//            ui->labelAddressLabelEdit->setText(tr("(no label)"));
    }
}

void RedirectDialog::on_viewButton_clicked()
{
    std::pair<std::string, int> pMultiSend;
    std::string strPrint;
    QString strStatus;
    if (pwalletMain->isRedirectNMRewardsEnabled()) {
        strStatus += tr("Redirect Active") + "\n";
    } else {
        strStatus += tr("Redirect Not Active") + "\n";
    }

    for (const auto& p: pwalletMain->mapMNRedirect) {
        if (model && model->getAddressTableModel()) {
            std::string associatedLabel;
            associatedLabel = model->getAddressTableModel()->labelForAddress(p.first.ToString().c_str()).toStdString();
            strPrint += associatedLabel.c_str();
            strPrint += " - ";
        }
        strPrint += p.first.ToString();
        strPrint += " -> ";
        strPrint += p.second.ToString();
        strPrint += "\n";
    }
    ui->message->setProperty("status", "ok");
    ui->message->style()->polish(ui->message);
    ui->message->setText(strStatus + QString(strPrint.c_str()));
}

void RedirectDialog::on_addButton_clicked()
{
    std::string strFromAddress = ui->redirectFromAddressEdit->text().toStdString();
    CBitcoinAddress fromAddress(strFromAddress);
    if (!fromAddress.IsValid()) {
        ui->message->setProperty("status", "error");
        ui->message->style()->polish(ui->message);
        ui->message->setText(tr("The entered address: %1 is invalid.\nPlease check the address and try again.").arg(ui->redirectFromAddressEdit->text()));
        ui->redirectFromAddressEdit->setFocus();
        return;
    }
    std::string strToAddress = ui->redirectToAddressEdit->text().toStdString();
    CBitcoinAddress toAddress(strToAddress);
    if (!toAddress.IsValid()) {
        ui->message->setProperty("status", "error");
        ui->message->style()->polish(ui->message);
        ui->message->setText(tr("The entered address: %1 is invalid.\nPlease check the address and try again.").arg(ui->redirectToAddressEdit->text()));
        ui->redirectToAddressEdit->setFocus();
        return;
    }
    const auto it = pwalletMain->mapMNRedirect.find(fromAddress);
    if (it != pwalletMain->mapMNRedirect.end()) {
        if (it->second == toAddress) {
            ui->message->setProperty("status", "error");
            ui->message->style()->polish(ui->message);
            ui->message->setText(tr("This pair of addresses is already present."));
            return;
        }
        CWalletDB walletdb(pwalletMain->strWalletFile);
        if (!walletdb.EraseMNRedirect(strFromAddress)) {
            ui->message->setProperty("status", "error");
            ui->message->style()->polish(ui->message);
            ui->message->setText(tr("Failed to remove address pair from the database."));
            return;
        }
    }
    pwalletMain->mapMNRedirect[fromAddress] = toAddress;
    ui->message->setProperty("status", "ok");
    ui->message->style()->polish(ui->message);
    std::string strPrint;
    for (const auto& p: pwalletMain->mapMNRedirect) {
        strPrint += p.first.ToString();
        strPrint += " -> ";
        strPrint += p.second.ToString();
        strPrint += "\n";
    }

//    if (model && model->getAddressTableModel()) {
//        // update the address book with the label given or no label if none was given.
//        CBitcoinAddress address(strAddress);
//        std::string userInputLabel = ui->labelAddressLabelEdit->text().toStdString();
//        if (!userInputLabel.empty())
//            model->updateAddressBookLabels(address.Get(), userInputLabel, "send");
//        else
//            model->updateAddressBookLabels(address.Get(), "(no label)", "send");
//    }

    CWalletDB walletdb(pwalletMain->strWalletFile);
    if(!walletdb.WriteMNRedirect(strFromAddress, strToAddress)) {
        ui->message->setProperty("status", "error");
        ui->message->style()->polish(ui->message);
        ui->message->setText(tr("Saved the Redirect to memory, but failed saving properties to the database."));
        return;
    }
    ui->message->setText(tr("Redirect Map") + "\n" + QString(strPrint.c_str()));
}

void RedirectDialog::on_deleteButton_clicked()
{
    std::string strFromAddress = ui->redirectFromAddressEdit->text().toStdString();
    CBitcoinAddress fromAddress(strFromAddress);
    if (!fromAddress.IsValid()) {
        ui->message->setProperty("status", "error");
        ui->message->style()->polish(ui->message);
        ui->message->setText(tr("The entered address: %1 is invalid.\nPlease check the address and try again.").arg(ui->redirectFromAddressEdit->text()));
        ui->redirectFromAddressEdit->setFocus();
        return;
    }
    const auto it = pwalletMain->mapMNRedirect.find(fromAddress);
    if (it == pwalletMain->mapMNRedirect.end()) {
        ui->message->setText(tr("Could not locate address"));
        return;
    }
    pwalletMain->mapMNRedirect.erase(it);
    CWalletDB walletdb(pwalletMain->strWalletFile);
    if (!walletdb.EraseMNRedirect(strFromAddress)) {
        ui->message->setProperty("status", "error");
        ui->message->style()->polish(ui->message);
        ui->message->setText(tr("Failed to remove address pair from the database."));
        return;
    }
    ui->message->setText(tr("Removed %1").arg(QString(strFromAddress.c_str())));
}

void RedirectDialog::on_activateButton_clicked()
{
    QString strRet;
    if (pwalletMain->mapMNRedirect.empty()) {
        strRet = tr("Unable to activate Redirect, addresses map is empty");
    } else if (pwalletMain->mapMNRedirect.begin()->first.IsValid()) {
        pwalletMain->setRedirectNMRewardsEnabled();
        strRet = tr("Redirect activated");
    } else
        strRet = tr("First Address Not Valid");
    ui->message->setProperty("status", "ok");
    ui->message->style()->polish(ui->message);
    ui->message->setText(strRet);
}

void RedirectDialog::on_disableButton_clicked()
{
    QString strRet;
    pwalletMain->setRedirectNMRewardsDisabled();
    strRet = tr("MultiSend deactivated");

    ui->message->setProperty("status", "");
    ui->message->style()->polish(ui->message);
    ui->message->setText(strRet);
}
