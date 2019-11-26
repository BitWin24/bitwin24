#pragma once

//#include "loguru.hpp"

#include "util.h"

extern int trace_call;

//#define FUNC_FUNC_LOG_TRACE() LOG_F(INFO, "I'm hungry for some %.3f!", 3.14159);
#define FUNC_LOG_TRACE() LogPrintf("trace %S %d\n", __FUNCTION__, trace_call++)
//std::string teststr = "hjyvgfjhvyj";
//#define FUNC_LOG_TRACE LogPrintStr(teststr);
//#define FUNC_LOG_TRACE ;
