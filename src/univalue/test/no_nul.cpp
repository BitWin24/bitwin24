#include "/home/s/workspace/BitWin24/src/trace-log.h" //++++++++++++++++++
#include "univalue.h"

int main (int argc, char *argv[])
{
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++

    char buf[] = "___[1,2,3]___";
    UniValue val;
    return val.read(buf + 3, 7) ? 0 : 1;
}
