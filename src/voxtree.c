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
    unsigned dim;
    unsigned child_count;
    int use_allocator;
    alloc_t allocator;
    struct voxtree_n node;
};

size_t
voxtree_get_alloc_size(unsigned dimensions)
{
    return sizeof(struct voxtree_n)*(1 << dimensions);
}

voxtree_t *
voxtree_create(unsigned dimensions, unsigned depth, alloc_t *allocator)
{
    if(allocator && allocator->type == ALLOC_SYM)
        if(allocator->u.symmetric.size < sizeof(struct voxtree_n)*(1 << dimensions))
            return 0;//some idiot gave us a symmetric allocator that isn't big enough

    struct voxtree *ret;
    ret = malloc(sizeof(struct voxtree) + sizeof(unsigned long)*dimensions);
    ret->depth = depth;
    ret->dim = dimensions;
    ret->child_count = 1 << dimensions;

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
        struct voxtree_n *node_stack[tree->depth*tree->child_count + 1];
        unsigned node_stack_index = 1;
        node_stack[0] = &tree->node;

        while(node_stack_index > 0)
        {
            node = node_stack[node_stack_index-1];

            unsigned found_branch = 0;
            unsigned i;
            for(i=0; i<tree->child_count; i++)
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
voxtree_get(voxtree_t *tree, const unsigned long pos[])
{
    unsigned i;
    unsigned long width = 1 << tree->depth;

    for(i=0; i<tree->dim; i++)
        assert(pos[i] < width);

    register struct voxtree_n *node = &tree->node;

    while(!node->isleaf)
    {
        unsigned index;
        for(i=0, index=0; i<tree->dim; i++)
            index |= ((pos[i] & (width-1)) >= width/2) << i;

        node = &node->u.children[index];
        width >>= 1;
    }

    return node->u.data;
}

void
voxtree_set(voxtree_t *tree, const unsigned long pos[], void *data)
{
    unsigned i;
    unsigned long width = 1 << tree->depth;

    for(i=0; i<tree->dim; i++)
        assert(pos[i] < width);

    alloc_t allocator = tree->allocator;
    unsigned node_stack_index = 0;
    struct voxtree_n *node_stack[tree->depth];
    register struct voxtree_n *node = &tree->node;

    //step 1: find lowest leaf.
    while(!node->isleaf)
    {
        unsigned index;
        node_stack[node_stack_index++] = node;

        for(i=0, index=0; i<tree->dim; i++)
            index |= ((pos[i] & (width-1)) >= width/2) << i;

        node = &node->u.children[index];
        width >>= 1;
    }

    //step 2: return if already set correctly
    if(node->u.data == data)
        return;

    //step 3: create any new nodes (if necessicary)
    const unsigned d = tree->depth;
    while(node_stack_index < d)
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
                                                       sizeof(struct voxtree_n)*tree->child_count);
        else
            node->u.children = malloc(sizeof(struct voxtree_n)*tree->child_count);

        for(i=0; i<tree->child_count; i++)
            node->u.children[i] = tmp;

        node_stack[node_stack_index++] = node;

        unsigned index;
        for(i=0, index=0; i<tree->dim; i++)
            index |= ((pos[i] & (width-1)) >= width/2) << i;

        node = &node->u.children[index];
        width >>= 1;
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
        for(i=1; i<tree->child_count; i++)
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
voxtree_count_nodes_rec(struct voxtree_n *node, unsigned child_count)
{
    long long nodes = 1;
    unsigned i;

    if(!node->isleaf)
    for(i=0; i<child_count; i++)
        nodes += voxtree_count_nodes_rec(&node->u.children[i], child_count);

    return nodes;
}

long long
voxtree_count_nodes(voxtree_t *tree)
{
    long long nodes = 1;
    unsigned i;

    if(!tree->node.isleaf)
    for(i=0; i<tree->child_count; i++)
        nodes += voxtree_count_nodes_rec(&tree->node.u.children[i], tree->child_count);

    return nodes;
}
