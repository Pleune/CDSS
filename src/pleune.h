#ifndef PLEUNE_H
#define PLEUNE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mpool_dynamic mpool_dynamic_t;
typedef struct mpool_grow mpool_grow_t;
typedef struct mpool_static mpool_static_t;

enum plog_level {
    L_DEBUG = 1,
    L_INFO = 2,
    L_WARN = 3,
    L_ERROR = 4,
    L_FATAL = 5
};

enum plog_stream {
    S_PRIMARY = 0,
    S_SECONDARY = 1
};

typedef struct ringbuff ringbuff_t;

typedef struct threadpool tpool_t;
typedef void (*tpool_work_t)(void *);


mpool_dynamic_t *mpool_dynamic_create(size_t block_size, size_t object_size, size_t alignment);
void mpool_dynamic_destroy(mpool_dynamic_t *);
void *mpool_dynamic_alloc(mpool_dynamic_t *);
void mpool_dynamic_free(mpool_dynamic_t *, void *);
size_t mpool_dynamic_blocks(mpool_dynamic_t *);


mpool_grow_t *mpool_grow_create(size_t block_size, size_t object_size, size_t alignment);
void mpool_grow_destroy(mpool_grow_t *);
void *mpool_grow_alloc(mpool_grow_t *);
void mpool_grow_free(mpool_grow_t *, void *);


mpool_static_t *mpool_static_create(size_t pool_size, size_t object_size, size_t alignment);
static inline void mpool_static_destroy(mpool_static_t *m) {free(m);}
void *mpool_static_alloc(mpool_static_t *);
void mpool_static_free(mpool_static_t *, void *);


void plog(enum plog_level, const char *msg, ...);
void plog_set_level(enum plog_level);
void plog_set_stream(enum plog_stream s, FILE *);
void plog_flush();


ringbuff_t *ringbuff_create(size_t capacity);
static inline void ringbuff_destroy(ringbuff_t *r) {free(r);}
size_t ringbuff_remaining(ringbuff_t *);
size_t ringbuff_used(ringbuff_t *);
int ringbuff_empty(ringbuff_t *);
static inline int ringbuff_full(ringbuff_t *r) {return ringbuff_remaining(r) == 0;}
void ringbuff_put(ringbuff_t *, const void *mem, size_t bytes);
void ringbuff_remove(ringbuff_t *, void *mem, size_t bytes);


tpool_t *tpool_create(unsigned int threads);
void tpool_destroy(tpool_t *);
void tpool_add(tpool_t *, tpool_work_t, void *arg, int front);
void tpool_pause(tpool_t *);
void tpool_resume(tpool_t *);
void tpool_flush(tpool_t *);

#ifdef __cplusplus
}
#endif

#endif
