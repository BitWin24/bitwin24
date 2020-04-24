// Copyright (c) 2017 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITWIN24_QT_REDIRECTDIALOG_H
#define BITWIN24_QT_REDIRECTDIALOG_H

#include <QDialog>

namespace Ui
{
class RedirectDialog;
}

class WalletModel;
class QLineEdit;
class RedirectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RedirectDialog(QWidget* parent = nullptr);
    ~RedirectDialog();
    void setModel(WalletModel* model);
    void setFromAddress(const QString& address);
    void setToAddress(const QString& address);
    void setAddress(const QString& address, QLineEdit* addrEdit);
private slots:
    void on_viewButton_clicked();
    void on_addButton_clicked();
    void on_deleteButton_clicked();
    void on_activateButton_clicked();
    void on_disableButton_clicked();
    void on_fromAddressBookButton_clicked();
    void on_toAddressBookButton_clicked();

private:
    Ui::RedirectDialog* ui;
    WalletModel* model;
};

#endif // BITWIN24_QT_REDIRECTDIALOG_H
