#ifndef CDSS_MPOOL_STATIC_H
#define CDSS_MPOOL_STATIC_H

#include "alloc.h"

typedef struct mpool_static mpool_static_t;

mpool_static_t *mpool_static_create(size_t pool_size, size_t object_size, size_t alignment);
static inline void mpool_static_destroy(mpool_static_t *m) {free(m);}
void *mpool_static_alloc(mpool_static_t *);
void *mpool_static_calloc(mpool_static_t *);
void mpool_static_free(mpool_static_t *, void *);
cdss_alloc_t mpool_static_allocator(mpool_static_t *);

#endif
