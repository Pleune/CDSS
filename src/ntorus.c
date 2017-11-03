#include "cdss.h"

#include <math.h>

struct ntorus {
    size_t dimensions;
    ntorus_cb_t in, out;
    alloc_t allocator;
    void *default_value;

    union {
        size_t s;
        void *p;
    } flex_member[];
};

/*
 *    struct ntorus
 * +-------------------------+
 * | size_t dimensions       |
 * | size_t size[dimensions] |
 * | size_t pos[dimensions]  | //IMPORTANT: this ASSUMES that size_t
 * | DATA                    | //has the same or higher alignment
 * +-------------------------+ //requirements of void *
 */

static inline size_t *
size_segment(struct ntorus *n)
{
    return &n->flex_member[0].s;
}

static inline size_t *
pos_segment(struct ntorus *n)
{
    return &n->flex_member[n->dimensions].s;
}

static inline void **
data_segment(struct ntorus *n)
{
    return &n->flex_member[2*n->dimensions].p;
}

ntorus_t *
ntorus_create(size_t dimensions, const size_t size[], void *default_value, const alloc_t *allocator)
{
    size_t data_len = size[0];
    size_t i;

    for(i=1; i<dimensions; i++)
        data_len *= size[i];

    struct ntorus *ret;
    size_t alloc_size =
        sizeof(struct ntorus) +
        sizeof(ret->flex_member[0]) * (2*dimensions + data_len);

    if(allocator)
    {
        if(allocator->type == ALLOC_ASYM)
        {
            ret = allocator->u.asymmetric.alloc(allocator->u.asymmetric.argument,
                                                alloc_size);
        } else {
            if(allocator->u.symmetric.size >= alloc_size)
                ret = allocator->u.symmetric.alloc(allocator->u.symmetric.argument);
            else
                return 0;//symmetric allocator is not big enough...
        }
        ret->allocator = *allocator;
    } else {
        ret = malloc(alloc_size);
        ret->allocator.type = ALLOC_NONE;
    }

    ret->dimensions = dimensions;
    ret->in = ret->out = 0;
    ret->default_value = default_value;

    for(i=0; i<dimensions; i++)
        size_segment(ret)[i] = size[i];

    size_t b;
    for(b=0; b<data_len; b++)
        data_segment(ret)[b] = default_value;

    return ret;
}

inline void **
ntorus_at(ntorus_t *n, const size_t pos[])
{
    size_t index = 0;
    size_t i;
    size_t mux;

    for(i = 0, mux = 1; i<n->dimensions; i++)
    {
        index += MODULO(pos[i], size_segment(n)[i]) * mux;
        mux *= size_segment(n)[i];
    }

    return data_segment(n)+index;
}

void
ntorus_foreach(ntorus_t *n, const size_t low[], const size_t high[], ntorus_cb_t cb)
{
    size_t stack[n->dimensions];

    size_t i;
    for(i=0; i<n->dimensions; i++)
    {
        if(low[i] > high[i])
            return;
    }

    for(i=0; i<n->dimensions; i++)
        stack[i] = low[i];

    for(;;)
    {
        cb(n, ntorus_at(n, stack));

        i = n->dimensions-1;
        while(++stack[i] > high[i])
        {
            stack[i] = low[i];
            if(i == 0) return;
            i--;
        }
    }
}

void
ntorus_fill(ntorus_t *n, const size_t low[], const size_t high[], void *data)
{
    size_t stack[n->dimensions];

    size_t i;
    for(i=0; i<n->dimensions; i++)
    {
        if(low[i] > high[i])
            return;
    }

    for(i=0; i<n->dimensions; i++)
        stack[i] = low[i];

    for(;;)
    {
        *ntorus_at(n, stack) = data;

        i = n->dimensions-1;
        while(++stack[i] > high[i])
        {
            stack[i] = low[i];
            if(i == 0) return;
            i--;
        }
    }
}

void
ntorus_move(ntorus_t *n, const size_t pos[])
{
    unsigned i;
    size_t shift[n->dimensions];

    for(i = 0; i<n->dimensions; i++)
        shift[i] = pos[i] - pos_segment(n)[i];

    ntorus_shift(n, shift);
}

void
ntorus_shift(ntorus_t *n, const size_t diff[])
{
    unsigned i;
    int low[n->dimensions];
    int high[n->dimensions];

    for(i = 0; i<n->dimensions; i++)
    {
        low[i] = 0;
        high[i] = size_segment(n)[i];
    }

    for(i = 0; i<n->dimensions; i++)
    {
        unsigned j;
        size_t low_[n->dimensions];
        size_t high_[n->dimensions];

        int changed = MIN(diff[i], size_segment(n)[i]);
        high[i] = changed;

        for(j=0; j<n->dimensions; j++)
        {
            low_[j] = pos_segment(n)[j] + low[j];
            high_[j] = pos_segment(n)[j] + high[j] - 1;
        }

        if(n->out) ntorus_foreach(n, low_, high_, n->out);
        ntorus_fill(n, low_, high_, n->default_value);
        if(n->in) ntorus_foreach(n, low_, high_, n->in);

        low[i] = changed;
        high[i] = size_segment(n)[i];
    }

    for(i = 0; i<n->dimensions; i++)
        pos_segment(n)[i] += diff[i];
}

void
ntorus_pos(ntorus_t *n, size_t pos[])
{
    unsigned i;
    for(i=0; i<n->dimensions; i++)
        pos[i] = pos_segment(n)[i];
}

void
ntorus_set_default(ntorus_t *n, void *default_value)
{
    n->default_value = default_value;
}

void
ntorus_callback_out(ntorus_t *n, ntorus_cb_t func)
{
    n->out = func;
}

void
ntorus_callback_in(ntorus_t *n, ntorus_cb_t func)
{
    n->in = func;
}
