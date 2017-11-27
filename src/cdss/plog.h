#ifndef CDSS_PLOG_H
#define CDSS_PLOG_H

#include <stdio.h>

enum plog_level {
    L_DEBUG = 1,
    L_INFO = 2,
    L_WARN = 3,
    L_ERROR = 4,
    L_FATAL = 5
};

enum plog_stream {
    S_PRIMARY = 0,
    S_SECONDARY = 1
};

void plog(enum plog_level, const char *msg, ...);
void plog_set_level(enum plog_level);
void plog_set_stream(enum plog_stream s, FILE *);
void plog_flush();

#endif
