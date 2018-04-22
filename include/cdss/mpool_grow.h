#ifndef CDSS_MPOOL_GROW_H
#define CDSS_MPOOL_GROW_H

#include "alloc.h"

typedef struct mpool_gr mpool_gr_t;

mpool_gr_t *mpool_gr_create(size_t block_size, size_t object_size, size_t alignment);
void mpool_gr_destroy(mpool_gr_t *);
void *mpool_gr_alloc(mpool_gr_t *);
void *mpool_gr_calloc(mpool_gr_t *);
void mpool_gr_free(mpool_gr_t *, void *);
cdss_alloc_t mpool_gr_allocator(mpool_gr_t *);

#endif
