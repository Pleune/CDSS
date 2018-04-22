#include "cdss/mpool_dynamic.h"
#include "cdss/alloc.h"
#include "cdss/minmax.h"

#include <string.h>

struct mpool_dy {
    void *next_free;
    struct mpool_dy *next_block;
    size_t block_size;
    size_t object_size;
    size_t alignment;
    size_t num_objects;
};

#define OBJECT_SIZE(alignment, obj_size)                                \
    (MAX(sizeof(void *), obj_size) + (alignment -                       \
     MAX(sizeof(void *), obj_size))%alignment)

#define HEADER_SIZE(alignment, pointer)                                 \
    (sizeof(struct mpool_dy) + (alignment -                        \
    (sizeof(struct mpool_dy) + (size_t)pointer))%alignment)

mpool_dy_t *
mpool_dy_create(size_t block_size, size_t object_size, size_t alignment)
{
    if(alignment < 1)
        alignment = 1;

    mpool_dy_t *ret = malloc(block_size);
    if(!ret) return 0;

    void *this, *next;
    const void * const end = (void *)ret + block_size;

    object_size = OBJECT_SIZE(alignment, object_size);

    ret->next_free = this = next = ((void *)ret) + HEADER_SIZE(alignment, ret);
    ret->next_block = 0;

    ret->block_size = block_size;
    ret->object_size = object_size;
    ret->alignment = alignment;
    ret->num_objects = 0;

    while(next <= end - object_size)
    {
        this = next;
        next += object_size;
        *(void **)this = next;
    }
    *(void **)this = 0;

    return ret;
}

void
mpool_dy_destroy(mpool_dy_t *m)
{
    void *this = m;
    while(this)
    {
        void *next = ((struct mpool_dy *)this)->next_block;
        free(this);
        this = next;
    }
}

void *
mpool_dy_alloc(mpool_dy_t *m)
{
    void *ret;

 mpool_dy_cdss_alloc_top:

    ret = m->next_free;
    if(ret == 0)
    {
        if(!m->next_block)
            m->next_block = mpool_dy_create(m->block_size, m->object_size, m->alignment);

        m = m->next_block;
        goto mpool_dy_cdss_alloc_top;
    }

    m->num_objects++;
    m->next_free = *(void **)ret;
    return ret;
}

void *
mpool_dy_calloc(mpool_dy_t *m)
{
    return memset(mpool_dy_alloc(m), 0, m->object_size);
}

void
mpool_dy_free(mpool_dy_t *m, void *p)
{
    struct mpool_dy *m_ = m;
    struct mpool_dy *m_prev;

    void *end = (void *)m + m->block_size;
    while(!(p > (void *)m && p < end))
    {
        m_prev = m;
        m = m->next_block;
        end = (void *)m + m->block_size;
    }

    m->num_objects--;

    if(m != m_ && m->num_objects == 0)
    {
        m_prev->next_block = m->next_block;
        free(m);
    } else {
        *(void **)p = m->next_free;
        m->next_free = p;
    }
}

size_t
mpool_dy_blocks(mpool_dy_t *m)
{
    void *this = m;
    size_t i = 0;

    while(this)
    {
        void *next = ((struct mpool_dy *)this)->next_block;
        i++;
        this = next;
    }

    return i;
}

cdss_alloc_t
mpool_dy_allocator(mpool_dy_t *m)
{
    static cdss_alloc_t ret = {
        {
            .symmetric = {
                (void *(*)(void *))&mpool_dy_alloc,
                (void *(*)(void *))&mpool_dy_calloc,
                (void (*)(void *, void *))&mpool_dy_free,
                0, 0
            }
        },
        ALLOC_SYM
    };

    ret.u.symmetric.argument = m;
    ret.u.symmetric.size = m->object_size;

    return ret;
}
