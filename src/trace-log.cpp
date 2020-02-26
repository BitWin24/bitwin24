//
// Created by s on 13.11.2019.
//

#include "trace-log.h"
#include <ctime>


int trace_call = 0;

//loguru::init(argc, argv);
//loguru::add_file("trace_log.log", loguru::Append, loguru::Verbosity_MAX);


void WriteToFile(const char *function) {
    char filename[256];
    sprintf(filename, "thread_%d.log", pthread_self());
    FILE *f = fopen(filename, "a+");
    if (f) {
        char message[2000];
        time_t now = time(0);
        struct tm *timeinfo;
        timeinfo = localtime(&now);
        char timeBuf[256];
        strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", timeinfo);
        sprintf(message, "%s: %s %d\n", timeBuf, function, trace_call++);
        fwrite(message, strlen(message), 1, f);
        std::fclose(f);
    }
}