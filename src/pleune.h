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

typedef struct stack stack_t;

typedef struct threadpool tpool_t;
typedef void (*tpool_work_t)(void *);

typedef struct voxtree voxtree_t;


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


stack_t *stack_create(size_t object_size, size_t object_count, double resize_factor);
void stack_destroy(stack_t *stack);
void stack_push(stack_t *stack, const void *data);
void *stack_pop(stack_t *stack, void *data);
size_t stack_objects_get_num(stack_t *stack);
void stack_push_mult(stack_t *stack, const void *data, size_t count);
void stack_advance(stack_t *stack, size_t count);
void stack_trim(stack_t *stack);
void stack_resize(stack_t *stack, size_t num_elements);
void stack_ensure_size(stack_t *stack, size_t num_elements);
void *stack_element_ref(stack_t *stack, size_t index);
//delete index by replacing it with the last element
//returns the ref to index, or 0 on error
void *stack_element_replace_from_end(stack_t *stack, size_t index);
void *stack_transform_dataptr(stack_t *stack);


tpool_t *tpool_create(unsigned int threads);
void tpool_destroy(tpool_t *);
void tpool_add(tpool_t *, tpool_work_t, void *arg, int front);
void tpool_pause(tpool_t *);
void tpool_resume(tpool_t *);
void tpool_flush(tpool_t *);


/* If alloc_func and free_func are suppplied, then alloc func is called
 * with the single argument func_arg1 to allocate a block of the
 * correct size (see: voxtree_get_alloc_size). The function free_func
 * is called with the first argument func_arg1, and the second
 * argument the pointer returned by alloc_func to free these blocks of
 * data.
 *
 * This is intended to be used with the mpool objects above, but any
 * simmilar functions may be used.
 */
voxtree_t *voxtree_create(unsigned depth,
                          void *(*alloc_func)(void *),
                          void (*free_func)(void *, void *),
                          void *func_arg1);
void voxtree_destroy(voxtree_t *);
void *voxtree_get(voxtree_t *tree,
                 unsigned long x,
                 unsigned long y,
                  unsigned long z);
void voxtree_set(voxtree_t *tree,
                 unsigned long x,
                 unsigned long y,
                 unsigned long z,
                 void *data);
size_t voxtree_get_alloc_size();
long long voxtree_count_nodes(voxtree_t *tree);


#ifdef __cplusplus
}
#endif

#endif
