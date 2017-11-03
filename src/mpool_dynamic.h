#ifndef CDSS_MPOOL_DYNAMIC_H
#define CDSS_MPOOL_DYNAMIC_H

#include "alloc.h"

typedef struct mpool_dynamic mpool_dynamic_t;

mpool_dynamic_t *mpool_dynamic_create(size_t block_size, size_t object_size, size_t alignment);
void mpool_dynamic_destroy(mpool_dynamic_t *);
void *mpool_dynamic_alloc(mpool_dynamic_t *);
void *mpool_dynamic_calloc(mpool_dynamic_t *);
void mpool_dynamic_free(mpool_dynamic_t *, void *);
size_t mpool_dynamic_blocks(mpool_dynamic_t *);
alloc_t mpool_dynamic_allocator(mpool_dynamic_t *);

#endif
