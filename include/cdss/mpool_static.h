#ifndef CDSS_MPOOL_STATIC_H
#define CDSS_MPOOL_STATIC_H

#include "alloc.h"

typedef struct mpool_st mpool_st_t;

mpool_st_t *mpool_st_create(size_t pool_size, size_t object_size, size_t alignment);
static inline void mpool_st_destroy(mpool_st_t *m) {free(m);}
void *mpool_st_alloc(mpool_st_t *);
void *mpool_st_calloc(mpool_st_t *);
void mpool_st_free(mpool_st_t *, void *);
cdss_alloc_t mpool_st_allocator(mpool_st_t *);

#endif
