#ifndef CDSS_VOXTREE_H
#define CDSS_VOXTREE_H

#include "cdss/alloc.h"
#include "cdss/types.h"
#include <stdlib.h>

typedef struct voxtree voxtree_t;

typedef struct {
    cdss_integer_t data; // only filled if leaf
    long            bounds[]; // 2 * dimensions
} voxtree_region_t;

typedef void (*voxtree_node_cb)(int is_leaf, const voxtree_region_t *region);

voxtree_t *     voxtree_create(unsigned dimensions, unsigned depth, const alloc_t *allocator, cdss_integer_t inital);
void            voxtree_destroy(voxtree_t *);
cdss_integer_t voxtree_get(voxtree_t *tree, const unsigned long pos[]);
void            voxtree_set(voxtree_t *tree, const unsigned long pos[], cdss_integer_t data);
size_t          voxtree_get_alloc_size(unsigned dimensions);
long long       voxtree_count_nodes(voxtree_t *tree);
void            voxtree_iterate_nodes(voxtree_t *tree, voxtree_node_cb);

#endif
