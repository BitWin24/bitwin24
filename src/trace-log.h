#pragma once

//#include "loguru.hpp"

#include "util.h"
#include <pthread.h>
#include <stdio.h>

extern int trace_call;


//#define FUNC_LOG_TRACE() LogPrintf("trace %S %d\n", __FUNCTION__, trace_call++)

void WriteToFile(const char* function);

#define FUNC_LOG_TRACE() WriteToFile(__PRETTY_FUNCTION__)