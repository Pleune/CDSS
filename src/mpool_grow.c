#include "cdss.h"

#include <string.h>
#include <assert.h>

/*
 *      Memory Diagram
 *
 *  ^ +-----------------+      // next_free is a singly linked list
 *  | | HEADER          |      // containing only pointers. They are
 *  | |  next_block* --------+ // overwriten when the slot is used.
 *  B |  next_free* --+ |    | //
 *  L |  object_size  | |    | // On allocation, next_free moves
 *  O |  block_size   | |    | // forward in the list.
 *  C |  alignment    | |    | // On free, the freed element is
 *  K | DATA          | |    | // put at the front of the next_free
 *  S |  union {      | |    | // list
 *  I |   next_free*  | |    | //
 *  Z |   obj_data    | |    | // As elements are free'd, the next_free
 *  E |  } objs[]<----+ |    | // list may jump erraticly between blocks.
 *  | |                 |    | // This makes a reduction in blocks slow,
 *  | |                 |    | // but free and alloc are very fast.
 *  v +-----------------+    |
 *                           |
 *  ^ +-----------------+ <--+
 *  | | HEADER          |
 *  | |  next_block* --------+
 *  B |  next_free* --+ |    |
 *  L |  object_size  | |    |
 *  O |  block_size   | |    |
 *  C |  alignment    | |    |
 *  K | DATA          | |    |
 *  S |  union {      | |    |
 *  I |   next_free*  | |    |
 *  Z |   obj_data    | |    |
 *  E |  } objs[]<----+ |    |
 *  | |                 |    |
 *  | |                 |    |
 *  v +-----------------+    |
 *                           V
 */

#define HEADER_SIZE(alignment, pointer)                                 \
    (sizeof(struct mpool_grow) + (alignment -                           \
     (sizeof(struct mpool_grow) + (size_t)pointer))%alignment)

#define OBJECT_SIZE(alignment, obj_size)                                \
    (MAX(sizeof(void *), obj_size) + (alignment -                       \
     MAX(sizeof(void *), obj_size))%alignment)

struct mpool_grow {
    void *next_block;

    //vvv only used in first block vvv
    void *next_free;
    size_t object_size;
    size_t block_size;
    size_t alignment;
};

mpool_grow_t *
mpool_grow_create(size_t block_size, size_t object_size, size_t alignment)
{
    if(alignment < 1)
        alignment = 1;

    object_size = OBJECT_SIZE(alignment, object_size);
    assert(block_size >= sizeof(struct mpool_grow) + object_size + alignment);

    void *ret = malloc(block_size);
    const void * const end = ret + block_size;
    void *next, *this;

    ((struct mpool_grow *)ret)->object_size = object_size;
    ((struct mpool_grow *)ret)->block_size = block_size;
    ((struct mpool_grow *)ret)->alignment = alignment;
    ((struct mpool_grow *)ret)->next_block = 0;
    ((struct mpool_grow *)ret)->next_free = next = this = ret + HEADER_SIZE(alignment, ret);

    //build free list
    while(next <= end - object_size)//next has room to be an object
    {
        this = next;
        next += object_size;
        *(void **)this = next;
    }

    //last objs[].next = 0
    *(void **)this = 0;

    return ret;
}

void
mpool_grow_destroy(mpool_grow_t *m)
{
    void *this = m;
    while(this)
    {
        void *next = ((struct mpool_grow *)this)->next_block;
        free(this);
        this = next;
    }
}

void *
mpool_grow_alloc(mpool_grow_t *m)
{
    void *ret = ((struct mpool_grow *)m)->next_free;

    if(ret == 0)
    {
        size_t object_size = ((struct mpool_grow *)m)->object_size;
        size_t block_size = ((struct mpool_grow *)m)->block_size;
        size_t alignment = ((struct mpool_grow *)m)->alignment;

        void *newblock = mpool_grow_create(block_size, object_size, alignment);

        ((struct mpool_grow *)newblock)->next_block = ((struct mpool_grow *)m)->next_block;
        ((struct mpool_grow *)m)->next_block = newblock;

        ret = ((struct mpool_grow *)newblock)->next_free;
    }

    ((struct mpool_grow *)m)->next_free = *(void **)(ret);
    return ret;
}

void *
mpool_grow_calloc(mpool_grow_t *m)
{
    return memset(mpool_grow_alloc(m), 0, m->object_size);
}

void
mpool_grow_free(mpool_grow_t *m, void *p)
{
    *(void **)p = ((struct mpool_grow *)m)->next_free;
    ((struct mpool_grow *)m)->next_free = p;
}

alloc_t
mpool_grow_allocator(mpool_grow_t *m)
{
    static alloc_t ret = {
        {
            .symmetric = {
                (void *(*)(void *))&mpool_grow_alloc,
                (void *(*)(void *))&mpool_grow_calloc,
                (void (*)(void *, void *))&mpool_grow_free,
                0, 0
            }
        },
        ALLOC_SYM
    };

    ret.u.symmetric.argument = m;
    ret.u.symmetric.size = m->object_size;

    return ret;
}
