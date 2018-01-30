#ifndef CDSS_BASETYPE_H
#define CDSS_BASETYPE_H

typedef union {
    void *        pointer;
    int           sint;
    unsigned      uint;
    long          slong;
    unsigned long ulong;
} cdss_integer_t;

#define CDSS_INT_CMP_ALL(a, b)                                                                     \
    (((a).sint == (b).sint && (a).uint == (b).uint && (a).slong == (b).slong                       \
      && (a).ulong == (b).ulong && (a).pointer == (b).pointer))

#endif
