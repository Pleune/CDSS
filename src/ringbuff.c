#include "cdss/ringbuff.h"
#include "cdss/minmax.h"

#include <string.h>
#include <assert.h>

struct ringbuff {
    size_t capacity;
    unsigned char *head;
    unsigned char *tail;
    unsigned char *mem;
};

ringbuff_t *ringbuff_create(size_t capacity)
{
    //ret->mem is garenteed to point to at least capacity+1 bytes
    ringbuff_t *ret = malloc(sizeof(struct ringbuff) + capacity + 1);
    ret->head = ret->tail = ret->mem = (void *)ret + sizeof(struct ringbuff);

    ret->capacity = capacity;
    return ret;
}

size_t ringbuff_remaining(ringbuff_t *r)
{
    if(r->head >= r->tail)
        return r->capacity - (r->head - r->tail);
    else
        return r->tail - r->head - 1;
}

size_t ringbuff_used(ringbuff_t *r)
{
    if(r->head >= r->tail)
        return r->head - r->tail;
    else
        return r->capacity - (size_t)(r->tail - r->head - 1);
}

int ringbuff_empty(ringbuff_t *r)
{
    return r->head == r->tail;
}

void ringbuff_put(ringbuff_t *r, const void *mem, size_t bytes)
{
    unsigned char *buffend = r->mem + r->capacity;
    size_t numread = 0;
    while(numread != bytes)
    {
        size_t toread = MIN(bytes - numread, (size_t)(buffend - r->head));

        memcpy(r->head, mem + numread, toread);
        numread += toread;
        r->head += toread;

        if(r->head == buffend)
            r->head = r->mem;
    }
}

void ringbuff_remove(ringbuff_t *r, void *mem, size_t bytes)
{
    unsigned char *buffend = r->mem + r->capacity;
    size_t numremove = 0;
    while(numremove != bytes)
    {
        size_t toremove = MIN(bytes - numremove, (size_t)(buffend - r->tail));

        if(mem) memcpy(mem + numremove, r->tail, toremove);
        numremove += toremove;
        r->tail += toremove;

        if(r->tail == buffend)
            r->tail = r->mem;
    }
}
