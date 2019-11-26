//
// Created by s on 13.11.2019.
//

#include "trace-log.h"


int trace_call = 0;

//loguru::init(argc, argv);
//loguru::add_file("trace_log.log", loguru::Append, loguru::Verbosity_MAX);


void WriteToFile(const char* function)
{
    char filename[256];
    sprintf(filename, "/home/s/my%d.log", pthread_self());
    FILE* f = fopen(filename, "a+");
    if(f) {
        char message[2000];
        sprintf(message, "trace %s %d\n", function, trace_call++);
        fwrite(message, strlen(message), 1, f);
        std::fclose(f);
    }
}