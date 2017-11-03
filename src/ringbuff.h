#ifndef CDSS_RINGBUFF_H
#define CDSS_RINGBUFF_H

#include <stdlib.h>

typedef struct ringbuff ringbuff_t;

ringbuff_t *ringbuff_create(size_t capacity);
static inline void ringbuff_destroy(ringbuff_t *r) {free(r);}
size_t ringbuff_remaining(ringbuff_t *);
size_t ringbuff_used(ringbuff_t *);
int ringbuff_empty(ringbuff_t *);
static inline int ringbuff_full(ringbuff_t *r) {return ringbuff_remaining(r) == 0;}
void ringbuff_put(ringbuff_t *, const void *mem, size_t bytes);
void ringbuff_remove(ringbuff_t *, void *mem, size_t bytes);

#endif
