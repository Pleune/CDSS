#include "pleune.h"

#include <stdlib.h>
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
    (sizeof(struct mempool) + (alignment -                              \
     (sizeof(struct mempool) + (size_t)pointer))%alignment)

#define OBJECT_SIZE(alignment, obj_size)                                \
    (MAX(sizeof(void *), obj_size) + (alignment -                       \
     MAX(sizeof(void *), obj_size))%alignment)

struct mempool {
    void *next_block;

    //vvv only used in first block vvv
    void *next_free;
    size_t object_size;
    size_t block_size;
    size_t alignment;
};

mempool_t *
mempool_create(size_t block_size, size_t object_size, size_t alignment)
{
    assert(alignment > 0);
    object_size = OBJECT_SIZE(alignment, object_size);
    assert(block_size >= sizeof(struct mempool) + object_size + alignment);

    void *ret = malloc(block_size);
    const void * const end = ret + block_size;
    void *next, *this;

    ((struct mempool *)ret)->object_size = object_size;
    ((struct mempool *)ret)->block_size = block_size;
    ((struct mempool *)ret)->alignment = alignment;
    ((struct mempool *)ret)->next_block = 0;
    ((struct mempool *)ret)->next_free = next = this = ret + HEADER_SIZE(alignment, ret);

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
mempool_destroy(mempool_t *m)
{
    void *this = m;
    while(this)
    {
        void *next = ((struct mempool *)this)->next_block;
        free(this);
        this = next;
    }
}

void *
mempool_alloc(mempool_t *m)
{
    void *ret = ((struct mempool *)m)->next_free;

    if(ret == 0)
    {
        size_t object_size = ((struct mempool *)m)->object_size;
        size_t block_size = ((struct mempool *)m)->block_size;
        size_t alignment = ((struct mempool *)m)->alignment;

        void *newblock = mempool_create(block_size, object_size, alignment);

        ((struct mempool *)newblock)->next_block = ((struct mempool *)m)->next_block;
        ((struct mempool *)m)->next_block = newblock;

        ret = ((struct mempool *)newblock)->next_free;
    }

    ((struct mempool *)m)->next_free = *(void **)(ret);
    return ret;
}

void
mempool_free(mempool_t *m, void *p)
{
    *(void **)p = ((struct mempool *)m)->next_free;
    ((struct mempool *)m)->next_free = p;
}
