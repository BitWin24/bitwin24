#include "trace-log.h" //++++++++++++++++++
// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2017 The PIVX developers
// Copyright (c) 2018 The MAC developers
// Copyright (c) 2019 The BITWIN24 developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "amount.h"

#include "tinyformat.h"

CFeeRate::CFeeRate(const CAmount& nFeePaid, size_t nSize)
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    if (nSize > 0)
        nSatoshisPerK = nFeePaid * 1000 / nSize;
    else
        nSatoshisPerK = 0;
}

CAmount CFeeRate::GetFee(size_t nSize) const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    CAmount nFee = nSatoshisPerK * nSize / 1000;

    if (nFee == 0 && nSatoshisPerK > 0)
        nFee = nSatoshisPerK;

    return nFee;
}

std::string CFeeRate::ToString() const
{

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
    return strprintf("%d.%08d BITWIN24/kB", nSatoshisPerK / COIN, nSatoshisPerK % COIN);
}
