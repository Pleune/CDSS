#include "pleune.h"

struct mpool_static {
    void *next_free;
};

#define OBJECT_SIZE(alignment, obj_size)                                \
    (MAX(sizeof(void *), obj_size) + (alignment -                       \
     MAX(sizeof(void *), obj_size))%alignment)

mpool_static_t *
mpool_static_create(size_t pool_size, size_t object_size, size_t alignment)
{
    mpool_static_t *ret = malloc(pool_size + sizeof(struct mpool_static) + alignment);
    if(!ret) return 0;

    void *this, *next;
    const void * const end = (void *)ret + pool_size + sizeof(struct mpool_static) + alignment;
    ret->next_free = this = next = ((void *)ret) + OBJECT_SIZE(alignment, sizeof(struct mpool_static));
    object_size = OBJECT_SIZE(alignment, object_size);

    while(this < end - object_size)
    {
        this = next;
        next += object_size;
        *(void **)this = next;
    }
    *(void **)this = 0;

    return ret;
}

void *
mpool_static_alloc(mpool_static_t *m)
{
    void *ret = m->next_free;
    if(ret == 0) return 0;
    m->next_free = *(void **)ret;
    return ret;
}

void
mpool_static_free(mpool_static_t *m, void *p)
{
    *(void **)p = m->next_free;
    m->next_free = p;
}
