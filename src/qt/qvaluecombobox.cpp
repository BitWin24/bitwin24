#include "/home/s/workspace/BitWin24/src/trace-log.h" //++++++++++++++++++
// Copyright (c) 2011-2013 The Bitcoin developers
// Copyright (c) 2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qvaluecombobox.h"

QValueComboBox::QValueComboBox(QWidget* parent) : QComboBox(parent), role(Qt::UserRole)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(handleSelectionChanged(int)));
}

QVariant QValueComboBox::value() const
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    return itemData(currentIndex(), role);
}

void QValueComboBox::setValue(const QVariant& value)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    setCurrentIndex(findData(value, role));
}

void QValueComboBox::setRole(int role)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    this->role = role;
}

void QValueComboBox::handleSelectionChanged(int idx)
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    emit valueChanged();
}
