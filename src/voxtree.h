#ifndef CDSS_VOXTREE_H
#define CDSS_VOXTREE_H

#include <stdlib.h>
#include "alloc.h"

typedef struct voxtree voxtree_t;

voxtree_t *voxtree_create(unsigned dimensions, unsigned depth, alloc_t *allocator);
void voxtree_destroy(voxtree_t *);
void *voxtree_get(voxtree_t *tree, const unsigned long pos[]);
void voxtree_set(voxtree_t *tree, const unsigned long pos[], void *data);
size_t voxtree_get_alloc_size(unsigned dimensions);
long long voxtree_count_nodes(voxtree_t *tree);

#endif
