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

    enum {ALLOC_NONE, ALLOC_SYM, ALLOC_ASYM} type;
} alloc_t;

#endif
