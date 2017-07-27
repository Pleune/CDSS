#include "pleune.h"

#include <string.h>
#include <assert.h>

struct voxtree_n {
    union {
        struct voxtree_n *children;
        void *data;
    } u;
    unsigned char isleaf;
};

struct voxtree {
    void *(*alloc_func)(void *);
    void (*free_func)(void *, void *);
    void *alloc_arg;
    unsigned depth;

    struct voxtree_n node;
};

size_t
voxtree_get_alloc_size()
{
    return sizeof(struct voxtree_n)*8;
}

voxtree_t *
voxtree_create(unsigned depth,
               void *(*alloc_func)(void *),
               void (*free_func)(void *, void *),
               void *func_arg1)
{
    struct voxtree *ret;
    ret = malloc(sizeof(struct voxtree));

    ret->alloc_func = alloc_func;
    ret->free_func = free_func;
    ret->alloc_arg = func_arg1;
    ret->depth = depth;

    ret->node.isleaf = 1;
    ret->node.u.data = 0;

    return ret;
}

void
voxtree_destroy(voxtree_t *tree)
{
    void (*free_func)(void *, void *) = tree->free_func;
    void *free_arg1 = tree->alloc_arg;

    if(!tree->node.isleaf)
    {
        struct voxtree_n *node = 0;
        struct voxtree_n *node_stack[tree->depth*8 + 1];
        unsigned node_stack_index = 1;
        node_stack[0] = &tree->node;

        while(node_stack_index > 0)
        {
            node = node_stack[node_stack_index-1];

            unsigned found_branch = 0;
            unsigned i;
            for(i=0; i<8; i++)
            {
                if(!node->u.children[i].isleaf)
                {
                    found_branch = 1;
                    node_stack[node_stack_index++] = &node->u.children[i];
                }
            }

            if(!found_branch)
            {
                if(free_func)
                    free_func(free_arg1, node->u.children);
                else
                    free(node->u.children);

                node->isleaf = 1;
                node_stack_index--;
            }
        }
    }

    free(tree);
}

void *
voxtree_get(voxtree_t *tree, unsigned long x, unsigned long y, unsigned long z)
{
    register unsigned long half_width = 1 << (tree->depth - 1);
    unsigned long width = 2*half_width;

    assert(x < width);
    assert(y < width);
    assert(z < width);

    register struct voxtree_n *node = &tree->node;
    register unsigned long not_width = ~width;

    while(!node->isleaf)
    {
        node = &node->u.children[
            ((x >= half_width)     ) |
            ((y >= half_width) << 1) |
            ((z >= half_width) << 2)];

        x = (x << 1) & not_width;
        y = (y << 1) & not_width;
        z = (z << 1) & not_width;
    }

    return node->u.data;
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

    register struct voxtree_n *node = &tree->node;
    register unsigned long not_width = ~width;

    //step 1: find lowest leaf.
    while(!node->isleaf)
    {
        node_stack[node_stack_index++] = node;
        node = &node->u.children[
            ((x >= half_width)     ) |
            ((y >= half_width) << 1) |
            ((z >= half_width) << 2)];

        x = (x << 1) & not_width;
        y = (y << 1) & not_width;
        z = (z << 1) & not_width;
    }

    //step 2: return if already set correctly
    if(node->u.data == data)
        return;

    //step 3: create any new nodes (if necessicary)
    while(node_stack_index < tree->depth)
    {
        struct voxtree_n tmp = *node;
        node->isleaf = 0;

        if(tree->alloc_func)
            node->u.children = tree->alloc_func(tree->alloc_arg);
        else
            node->u.children = malloc(sizeof(struct voxtree_n)*8);

        unsigned i;
        for(i=0; i<8; i++)
            node->u.children[i] = tmp;

        node_stack[node_stack_index++] = node;
        node = &node->u.children[
            ((x >= half_width)     ) |
            ((y >= half_width) << 1) |
            ((z >= half_width) << 2)];

        x = (x << 1) & not_width;
        y = (y << 1) & not_width;
        z = (z << 1) & not_width;
    }

    //step 4: actually set the data in the new leaf node
    node->u.data = data;

    //step 5: walk back up the tree, and compress any branch nodes
    //with all identical children into leaf nodes
    while(node_stack_index > 0)
    {
        node = node_stack[--node_stack_index];

        if(!node->u.children[0].isleaf)
            return;

        void *tmp = node->u.children[0].u.data;

        unsigned i;
        for(i=1; i<8; i++)
        {
            if(!node->u.children[i].isleaf)
                return;

            if(tmp != node->u.children[i].u.data)
                return;
        }

        if(tree->free_func)
            tree->free_func(tree->alloc_arg, node->u.children);
        else
            free(node->u.children);

        node->isleaf = 1;
        node->u.data = tmp;
    }
}

static inline long long
voxtree_count_nodes_rec(struct voxtree_n *node)
{
    long long nodes = 1;
    unsigned i;

    if(!node->isleaf)
    for(i=0; i<8; i++)
        nodes += voxtree_count_nodes_rec(&node->u.children[i]);

    return nodes;
}

long long
voxtree_count_nodes(voxtree_t *tree)
{
    long long nodes = 1;
    unsigned i;

    if(!tree->node.isleaf)
    for(i=0; i<8; i++)
        nodes += voxtree_count_nodes_rec(&tree->node.u.children[i]);

    return nodes;
}
