#ifndef CDSS_ALLOC_H
#define CDSS_ALLOC_H

#include <stdlib.h>

/*
 * A general structure to define a set of alloc/free functions.
 * It does not allow the use of any alloc functions, but a
 * wrapper for any allocator can be made.
 *
 * All allocators in this library will have a function to build
 * this struct. Furthermore, any function in this library that
 * asks for a alloc_t pointer will also accept a void pointer,
 * meaning that function should use malloc/calloc/free from
 * the standard library.
 *
 * Symmetric allocators are those which only return constantly sized
 * blocks
 * Asymmetric allocators return blocks of any size, like malloc/calloc
 * from the standard library
 */
enum alloc_type {ALLOC_NONE, ALLOC_SYM, ALLOC_ASYM};

typedef struct {
    union {
        struct {
            void *(*alloc)(void *arg);
            void *(*calloc)(void *arg);
            void (*free)(void *arg, void *);
            void *argument;
            size_t size;
        } symmetric;
        struct {
            void *(*alloc)(void *arg, size_t);
            void *(*calloc)(void *arg, size_t);
            void (*free)(void *arg, void *);
            void *argument;
        } asymmetric;
    } u;

    enum alloc_type type;
} alloc_t;

extern const alloc_t ALLOC_STDLIB;

inline static void *
cdss_malloc(const alloc_t *a, const size_t s)
{
    switch(a->type)
    {
    case ALLOC_SYM:
        return a->u.symmetric.alloc(a->u.symmetric.argument);
    case ALLOC_ASYM:
        return a->u.asymmetric.alloc(a->u.asymmetric.argument, s);
    case ALLOC_NONE:
        return malloc(s);
    }

    return 0;
}

inline static void *
cdss_calloc(const alloc_t *a, const size_t s)
{
    switch(a->type)
    {
    case ALLOC_SYM:
        return a->u.symmetric.calloc(a->u.symmetric.argument);
    case ALLOC_ASYM:
        return a->u.asymmetric.calloc(a->u.asymmetric.argument, s);
    default:
        return calloc(1, s);
    }

    return 0;
}

inline static void
cdss_free(const alloc_t *a, void *d)
{
    switch(a->type)
    {
    case ALLOC_SYM:
        a->u.symmetric.free(a->u.symmetric.argument, d);
        return;
    case ALLOC_ASYM:
        a->u.asymmetric.free(a->u.asymmetric.argument, d);
        return;
    case ALLOC_NONE:
        free(d);
        return;
    }
}

inline static unsigned
cdss_alloc_ensure(const alloc_t *a, const size_t s)
{
    if(a->type == ALLOC_SYM && a->u.symmetric.size < s)
        return 0;
    else
        return 1;
}

#endif
