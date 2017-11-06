#ifndef CDSS_BASETYPE_H
#define CDSS_BASETYPE_H

typedef union {
    void *pointer;
    int sint;
    unsigned uint;
    long slong;
    unsigned long ulong;
} cdss_integer_t;

#endif
