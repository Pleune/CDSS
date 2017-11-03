#ifndef CDSS_MPOOL_GROW_H
#define CDSS_MPOOL_GROW_H

#include "alloc.h"

typedef struct mpool_grow mpool_grow_t;

mpool_grow_t *mpool_grow_create(size_t block_size, size_t object_size, size_t alignment);
void mpool_grow_destroy(mpool_grow_t *);
void *mpool_grow_alloc(mpool_grow_t *);
void *mpool_grow_calloc(mpool_grow_t *);
void mpool_grow_free(mpool_grow_t *, void *);
alloc_t mpool_grow_allocator(mpool_grow_t *);

#endif
