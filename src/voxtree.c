#include "pleune.h"

#include <string.h>
#include <assert.h>

struct voxtree_n {
    unsigned char isleaf;
    void *children;
};

struct voxtree {
    void *(*alloc_func)(void *);
    void (*free_func)(void *, void *);
    void *alloc_arg;
    size_t alloc_size;
    size_t node_size;
    size_t data_size;
    unsigned depth;

    struct voxtree_n *node;
};

#define get_data_ptr(node_ptr) (&((struct voxtree_n *)(node_ptr))->children)

inline size_t
voxtree_get_alloc_size(size_t data_size)
{
    size_t node_size = MAX(sizeof(struct voxtree_n) - sizeof(struct voxtree_n *) + data_size,
                           sizeof(struct voxtree_n));
    return MAX(node_size*8, sizeof(struct voxtree));
}

voxtree_t *
voxtree_create(unsigned depth,
               void *(*alloc_func)(void *),
               void (*free_func)(void *, void *),
               void *func_arg1,
               size_t data_size)
{
    struct voxtree *ret;
    size_t alloc_size = voxtree_get_alloc_size(data_size);

    if(alloc_func && alloc_size >= sizeof(struct voxtree))
        ret = alloc_func(func_arg1);
    else
        ret = malloc(sizeof(struct voxtree));

    if(alloc_func)
        ret->node = alloc_func(func_arg1);
    else
        ret->node = malloc(alloc_size);

    ret->alloc_func = alloc_func;
    ret->free_func = free_func;
    ret->alloc_arg = func_arg1;
    ret->node_size = MAX(sizeof(struct voxtree_n) - sizeof(struct voxtree_n *) + data_size,
                         sizeof(struct voxtree_n));
    ret->alloc_size = alloc_size;
    ret->data_size = data_size;
    ret->depth = depth;

    ret->node->isleaf = 1;
    memset(get_data_ptr(ret->node), 0, data_size);

    return ret;
}

void
voxtree_destroy(voxtree_t *tree)
{
    void (*free_func)(void *, void *) = tree->free_func;
    void *free_arg1 = tree->alloc_arg;
    size_t node_size = tree->node_size;

    if(!tree->node->isleaf)
    {
        struct voxtree_n *node = 0;
        struct voxtree_n *node_stack[tree->depth*80 + 1];
        unsigned node_stack_index = 1;
        node_stack[0] = tree->node;

        while(node_stack_index > 0)
        {
            node = node_stack[node_stack_index-1];

            unsigned found_branch = 0;
            unsigned i;
            for(i=0; i<8; i++)
            {
                if(!((struct voxtree_n *)(node->children + i*node_size))->isleaf)
                {
                    found_branch = 1;
                    node_stack[node_stack_index++] = node->children + i*node_size;
                }
            }

            if(!found_branch)
            {
                if(free_func)
                    free_func(free_arg1, node->children);
                else
                    free(node->children);

                node->isleaf = 1;
                node_stack_index--;
            }
        }
    }

    if(free_func)
        free_func(free_arg1, tree->node);
    else
        free(tree->node);

    if(tree->free_func && tree->alloc_size >= sizeof(struct voxtree))
        tree->free_func(tree->alloc_arg, tree);
    else
        free(tree);
}

void
voxtree_get(voxtree_t *tree, unsigned long x, unsigned long y, unsigned long z, void *data)
{
    register unsigned long half_width = 1 << (tree->depth - 1);
    unsigned long width = 2*half_width;

    assert(x < width);
    assert(y < width);
    assert(z < width);

    register struct voxtree_n *node = tree->node;
    register size_t node_size = tree->node_size;
    register unsigned long not_width = ~width;

    while(!node->isleaf)
    {
        node = node->children + node_size* (
            ((x >= half_width)     ) |
            ((y >= half_width) << 1) |
            ((z >= half_width) << 2));

        x = (x << 1) & not_width;
        y = (y << 1) & not_width;
        z = (z << 1) & not_width;
    }

    memcpy(data, get_data_ptr(node), tree->data_size);
}

void
voxtree_set(voxtree_t *tree, unsigned long x, unsigned long y, unsigned long z, void *data)
{
    register unsigned long half_width = 1 << (tree->depth - 1);
    unsigned long width = 2*half_width;

    assert(x < width);
    assert(y < width);
    assert(z < width);

    unsigned node_stack_index = 0;
    struct voxtree_n *node_stack[tree->depth];

    register struct voxtree_n *node = tree->node;
    register unsigned long not_width = ~width;

    size_t node_size = tree->node_size;

    //step 1: find lowest leaf.
    while(!node->isleaf)
    {
        node_stack[node_stack_index++] = node;
        node = node->children + node_size* (
            ((x >= half_width)     ) |
            ((y >= half_width) << 1) |
            ((z >= half_width) << 2));

        x = (x << 1) & not_width;
        y = (y << 1) & not_width;
        z = (z << 1) & not_width;
    }

    //step 2: return if already set correctly
    if(memcmp(data, get_data_ptr(node), tree->data_size) == 0)
        return;

    //step 3: create any new nodes (if necessicary)
    while(node_stack_index < tree->depth)
    {
        node->isleaf = 0;

        unsigned char tmp[tree->node_size];
        memcpy(get_data_ptr(tmp), get_data_ptr(node), tree->data_size);
        ((struct voxtree_n *)tmp)->isleaf = 1;

        if(tree->alloc_func)
            node->children = tree->alloc_func(tree->alloc_arg);
        else
            node->children = malloc(tree->alloc_size);

        unsigned i;
        for(i=0; i<8; i++)
            memcpy(node->children + i*node_size, tmp, node_size);

        node_stack[node_stack_index++] = node;
        node = node->children + node_size* (
            ((x >= half_width)     ) |
            ((y >= half_width) << 1) |
            ((z >= half_width) << 2));

        x = (x << 1) & not_width;
        y = (y << 1) & not_width;
        z = (z << 1) & not_width;
    }

    //step 4: actually set the data in the new leaf node
    memcpy(get_data_ptr(node), data, tree->data_size);

    //step 5: walk back up the tree, and compress any branch nodes
    //with all identical children into leaf nodes
    while(node_stack_index > 0)
    {
        node = node_stack[--node_stack_index];

        if(!((struct voxtree_n *)(node->children))->isleaf)
            return;

        unsigned char tmp[tree->data_size];
        memcpy(tmp, get_data_ptr(node->children), tree->data_size);

        unsigned i;
        for(i=1; i<8; i++)
        {
            if(!((struct voxtree_n *)(node->children + node_size*i))->isleaf)
                return;

            if(memcmp(tmp, get_data_ptr(node->children + node_size*i), tree->data_size) != 0)
                break;
        }

        if(i == 8)
        {
            if(tree->free_func)
                tree->free_func(tree->alloc_arg, node->children);
            else
                free(node->children);

            node->isleaf = 1;
            memcpy(get_data_ptr(node), tmp, tree->data_size);
        }
    }
}

long long
count_nodes(struct voxtree_n *node, size_t node_size)
{
    long long nodes = 1;
    unsigned i;

    if(!node->isleaf)
    for(i=0; i<8; i++)
        nodes += count_nodes(node->children + i*node_size, node_size);

    return nodes;
}

long long
voxtree_count_nodes(voxtree_t *tree)
{
    long long nodes = 1;
    unsigned i;

    if(!tree->node->isleaf)
    for(i=0; i<8; i++)
        nodes += count_nodes(tree->node->children + i*tree->node_size, tree->node_size);

    return nodes;
}
