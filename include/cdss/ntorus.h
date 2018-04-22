#ifndef CDSS_NTORUS_H
#define CDSS_NTORUS_H

#include <stdlib.h>
#include "alloc.h"

typedef struct ntorus ntorus_t;
typedef void (*ntorus_cb_t)(ntorus_t *, void **data);

ntorus_t *ntorus_create(const size_t dimensions,
                        const size_t size[],
                        void *default_value,
                        const cdss_alloc_t *allocator);
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

#endif
