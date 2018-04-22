#ifndef CDSS_MPOOL_DYNAMIC_H
#define CDSS_MPOOL_DYNAMIC_H

#include "alloc.h"

typedef struct mpool_dy mpool_dy_t;

mpool_dy_t *mpool_dy_create(size_t block_size, size_t object_size, size_t alignment);
void mpool_dy_destroy(mpool_dy_t *);
void *mpool_dy_alloc(mpool_dy_t *);
void *mpool_dy_calloc(mpool_dy_t *);
void mpool_dy_free(mpool_dy_t *, void *);
size_t mpool_dy_blocks(mpool_dy_t *);
cdss_alloc_t mpool_dy_allocator(mpool_dy_t *);

#endif
