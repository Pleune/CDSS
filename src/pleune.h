#ifndef PLEUNE_H
#define PLEUNE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MODULO(a, b) (((a) % (b) + (b)) % (b))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mpool_dynamic mpool_dynamic_t;
typedef struct mpool_grow mpool_grow_t;
typedef struct mpool_static mpool_static_t;
typedef struct ntorus ntorus_t;
typedef void (*ntorus_cb_t)(ntorus_t *, void **data);
typedef struct ringbuff ringbuff_t;
typedef struct stack stack_t;
typedef struct threadpool tpool_t;
typedef void (*tpool_work_t)(void *);
typedef struct voxtree voxtree_t;

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


mpool_dynamic_t *mpool_dynamic_create(size_t block_size, size_t object_size, size_t alignment);
void mpool_dynamic_destroy(mpool_dynamic_t *);
void *mpool_dynamic_alloc(mpool_dynamic_t *);
void *mpool_dynamic_calloc(mpool_dynamic_t *);
void mpool_dynamic_free(mpool_dynamic_t *, void *);
size_t mpool_dynamic_blocks(mpool_dynamic_t *);
alloc_t mpool_dynamic_allocator(mpool_dynamic_t *);


mpool_grow_t *mpool_grow_create(size_t block_size, size_t object_size, size_t alignment);
void mpool_grow_destroy(mpool_grow_t *);
void *mpool_grow_alloc(mpool_grow_t *);
void *mpool_grow_calloc(mpool_grow_t *);
void mpool_grow_free(mpool_grow_t *, void *);
alloc_t mpool_grow_allocator(mpool_grow_t *);


mpool_static_t *mpool_static_create(size_t pool_size, size_t object_size, size_t alignment);
static inline void mpool_static_destroy(mpool_static_t *m) {free(m);}
void *mpool_static_alloc(mpool_static_t *);
void *mpool_static_calloc(mpool_static_t *);
void mpool_static_free(mpool_static_t *, void *);
alloc_t mpool_static_allocator(mpool_static_t *);


ntorus_t *ntorus_create(const size_t dimensions,
                        const size_t size[],
                        void *default_value,
                        const alloc_t *allocator);
static inline void ntorus_destroy(ntorus_t *n) {free(n);}
void **ntorus_at(ntorus_t *, const size_t pos[]);
void ntorus_foreach(ntorus_t *,
                    const size_t low[],
                    const size_t high[],
                    ntorus_cb_t cb);//inclusive
void ntorus_fill(ntorus_t *,
                 const size_t low[],
                 const size_t high[],
                 void *data);//inclusive
void ntorus_move(ntorus_t *, const size_t pos[]);
void ntorus_shift(ntorus_t *, const size_t diff[]);
void ntorus_pos(ntorus_t *, size_t pos[]);
void ntorus_set_default(ntorus_t *, void *default_value);
void ntorus_callback_out(ntorus_t *, ntorus_cb_t func);
void ntorus_callback_in(ntorus_t *, ntorus_cb_t func);


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


voxtree_t *voxtree_create(unsigned dimensions, unsigned depth, alloc_t *allocator);
void voxtree_destroy(voxtree_t *);
void *voxtree_get(voxtree_t *tree, const unsigned long pos[]);
void voxtree_set(voxtree_t *tree, const unsigned long pos[], void *data);
size_t voxtree_get_alloc_size(unsigned dimensions);
long long voxtree_count_nodes(voxtree_t *tree);


#ifdef __cplusplus
}
#endif

#endif
