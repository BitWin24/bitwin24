#include "/home/s/workspace/BitWin24/src/trace-log.h" //++++++++++++++++++
// Copyright (c) 2018 The PIVX developers
// Copyright (c) 2018 The MAC developers
// Copyright (c) 2019 The BITWIN24 developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "invalid.h"
#include "invalid_outpoints.json.h"
#include "invalid_serials.json.h"

namespace invalid_out
{
    std::set<CBigNum> setInvalidSerials;
    std::set<COutPoint> setInvalidOutPoints;

    UniValue read_json(const std::string& jsondata)
    {
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

        UniValue v;

        if (!v.read(jsondata) || !v.isArray())
        {
            return UniValue(UniValue::VARR);
        }
        return v.get_array();
    }

    bool LoadOutpoints()
    {
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

        UniValue v = read_json(LoadInvalidOutPoints());

        if (v.empty())
            return false;

        for (unsigned int idx = 0; idx < v.size(); idx++) {
            const UniValue &val = v[idx];
            const UniValue &o = val.get_obj();

            const UniValue &vTxid = find_value(o, "txid");
            if (!vTxid.isStr())
                return false;

            uint256 txid = uint256(vTxid.get_str());
            if (txid == 0)
                return false;

            const UniValue &vN = find_value(o, "n");
            if (!vN.isNum())
                return false;

            auto n = static_cast<uint32_t>(vN.get_int());
            COutPoint out(txid, n);
            setInvalidOutPoints.insert(out);
        }
        return true;
    }

    bool LoadSerials()
    {
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

        UniValue v = read_json(LoadInvalidSerials());

        if (v.empty())
            return false;

        for (unsigned int idx = 0; idx < v.size(); idx++) {
            const UniValue &val = v[idx];
            const UniValue &o = val.get_obj();

            const UniValue &vSerial = find_value(o, "s");
            if (!vSerial.isStr())
                return false;

            CBigNum bnSerial = 0;
            bnSerial.SetHex(vSerial.get_str());
            if (bnSerial == 0)
                return false;
            setInvalidSerials.insert(bnSerial);
        }

        return true;
    }

    bool ContainsOutPoint(const COutPoint& out)
    {
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

        return static_cast<bool>(setInvalidOutPoints.count(out));
    }

    bool ContainsSerial(const CBigNum& bnSerial)
    {
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

        return static_cast<bool>(setInvalidSerials.count(bnSerial));
    }
}

