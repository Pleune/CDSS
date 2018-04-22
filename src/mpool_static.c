#include "cdss/mpool_static.h"
#include "cdss/minmax.h"

#include <string.h>

struct mpool_st {
    void *next_free;
    size_t object_size;
};

#define OBJECT_SIZE(alignment, obj_size)                                \
    (MAX(sizeof(struct mpool_st), obj_size) + (alignment -          \
     MAX(sizeof(struct mpool_st), obj_size))%alignment)

mpool_st_t *
mpool_st_create(size_t pool_size, size_t object_size, size_t alignment)
{
    if(alignment < 1)
        alignment = 1;

    mpool_st_t *ret = malloc(pool_size + sizeof(struct mpool_st) + alignment);
    if(!ret) return 0;

    void *this, *next;
    const void * const end = (void *)ret + pool_size + sizeof(struct mpool_st) + alignment;
    ret->next_free = this = next = ((void *)ret) + OBJECT_SIZE(alignment, sizeof(struct mpool_st));
    ret->object_size = object_size;
    object_size = OBJECT_SIZE(alignment, object_size);

    while(next <= end - object_size)
    {
        this = next;
        next += object_size;
        *(void **)this = next;
    }
    *(void **)this = 0;

    return ret;
}

void *
mpool_st_alloc(mpool_st_t *m)
{
    void *ret = m->next_free;
    if(ret == 0) return 0;
    m->next_free = *(void **)ret;
    return ret;
}

void *
mpool_st_calloc(mpool_st_t *m)
{
    return memset(mpool_st_alloc(m), 0, m->object_size);
}

void
mpool_st_free(mpool_st_t *m, void *p)
{
    *(void **)p = m->next_free;
    m->next_free = p;
}

cdss_alloc_t
mpool_st_allocator(mpool_st_t *m)
{
    static cdss_alloc_t ret = {
        {
            .symmetric = {
                (void *(*)(void *))&mpool_st_alloc,
                (void *(*)(void *))&mpool_st_calloc,
                (void (*)(void *, void *))&mpool_st_free,
                0, 0
            }
        },
        ALLOC_SYM
    };

    ret.u.symmetric.argument = m;
    ret.u.symmetric.size = m->object_size;

    return ret;
}
