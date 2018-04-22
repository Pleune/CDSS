#include "cdss/mpool_grow.h"
#include "cdss/minmax.h"

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
    (sizeof(struct mpool_gr) + (alignment -                           \
     (sizeof(struct mpool_gr) + (size_t)pointer))%alignment)

#define OBJECT_SIZE(alignment, obj_size)                                \
    (MAX(sizeof(void *), obj_size) + (alignment -                       \
     MAX(sizeof(void *), obj_size))%alignment)

struct mpool_gr {
    void *next_block;

    //vvv only used in first block vvv
    void *next_free;
    size_t object_size;
    size_t block_size;
    size_t alignment;
};

mpool_gr_t *
mpool_gr_create(size_t block_size, size_t object_size, size_t alignment)
{
    if(alignment < 1)
        alignment = 1;

    object_size = OBJECT_SIZE(alignment, object_size);
    assert(block_size >= sizeof(struct mpool_gr) + object_size + alignment);

    void *ret = malloc(block_size);
    const void * const end = ret + block_size;
    void *next, *this;

    ((struct mpool_gr *)ret)->object_size = object_size;
    ((struct mpool_gr *)ret)->block_size = block_size;
    ((struct mpool_gr *)ret)->alignment = alignment;
    ((struct mpool_gr *)ret)->next_block = 0;
    ((struct mpool_gr *)ret)->next_free = next = this = ret + HEADER_SIZE(alignment, ret);

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
mpool_gr_destroy(mpool_gr_t *m)
{
    void *this = m;
    while(this)
    {
        void *next = ((struct mpool_gr *)this)->next_block;
        free(this);
        this = next;
    }
}

void *
mpool_gr_alloc(mpool_gr_t *m)
{
    void *ret = ((struct mpool_gr *)m)->next_free;

    if(ret == 0)
    {
        size_t object_size = ((struct mpool_gr *)m)->object_size;
        size_t block_size = ((struct mpool_gr *)m)->block_size;
        size_t alignment = ((struct mpool_gr *)m)->alignment;

        void *newblock = mpool_gr_create(block_size, object_size, alignment);

        ((struct mpool_gr *)newblock)->next_block = ((struct mpool_gr *)m)->next_block;
        ((struct mpool_gr *)m)->next_block = newblock;

        ret = ((struct mpool_gr *)newblock)->next_free;
    }

    ((struct mpool_gr *)m)->next_free = *(void **)(ret);
    return ret;
}

void *
mpool_gr_calloc(mpool_gr_t *m)
{
    return memset(mpool_gr_alloc(m), 0, m->object_size);
}

void
mpool_gr_free(mpool_gr_t *m, void *p)
{
    *(void **)p = ((struct mpool_gr *)m)->next_free;
    ((struct mpool_gr *)m)->next_free = p;
}

cdss_alloc_t
mpool_gr_allocator(mpool_gr_t *m)
{
    static cdss_alloc_t ret = {
        {
            .symmetric = {
                (void *(*)(void *))&mpool_gr_alloc,
                (void *(*)(void *))&mpool_gr_calloc,
                (void (*)(void *, void *))&mpool_gr_free,
                0, 0
            }
        },
        ALLOC_SYM
    };

    ret.u.symmetric.argument = m;
    ret.u.symmetric.size = m->object_size;

    return ret;
}
