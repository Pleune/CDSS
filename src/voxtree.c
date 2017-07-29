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
    unsigned depth;
    int use_allocator;
    alloc_t allocator;
    struct voxtree_n node;
};

size_t
voxtree_get_alloc_size()
{
    return sizeof(struct voxtree_n)*8;
}

voxtree_t *
voxtree_create(unsigned depth, alloc_t *allocator)
{
    if(allocator && allocator->type == ALLOC_SYM)
        if(allocator->u.symmetric.size < sizeof(struct voxtree_n)*8)
            return 0;//some idiot gave us a symmetric allocator that isn't big enough

    struct voxtree *ret;
    ret = malloc(sizeof(struct voxtree));
    ret->depth = depth;

    if(allocator)
    {
        ret->allocator = *allocator;
        ret->use_allocator = 1;
    } else {
        ret->use_allocator = 0;
    }

    ret->node.isleaf = 1;
    ret->node.u.data = 0;

    return ret;
}

void
voxtree_destroy(voxtree_t *tree)
{
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
                if(tree->use_allocator)
                    if(tree->allocator.type == ALLOC_SYM)
                        tree->allocator.u.symmetric.free(tree->allocator.u.symmetric.argument,
                                                         node->u.children);
                    else
                        tree->allocator.u.asymmetric.free(tree->allocator.u.asymmetric.argument,
                                                         node->u.children);
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

    assert(x < 2*half_width);//assert improves performance
    assert(y < 2*half_width);
    assert(z < 2*half_width);

    alloc_t allocator = tree->allocator;

    unsigned node_stack_index = 0;
    struct voxtree_n *node_stack[tree->depth];

    register struct voxtree_n *node = &tree->node;
    register unsigned long not_width = ~(2*half_width);

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

        if(tree->use_allocator)
            if(allocator.type == ALLOC_SYM)
                node->u.children =
                    allocator.u.symmetric.alloc(allocator.u.symmetric.argument);
            else
                node->u.children =
                    allocator.u.asymmetric.alloc(allocator.u.asymmetric.argument,
                                                       sizeof(struct voxtree_n)*8);
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

        if(tree->use_allocator)
            if(allocator.type == ALLOC_SYM)
                allocator.u.symmetric.free(allocator.u.symmetric.argument,
                                                 node->u.children);
            else
                allocator.u.asymmetric.free(allocator.u.asymmetric.argument,
                                                  node->u.children);
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
