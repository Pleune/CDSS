#include "cdss/voxtree.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

struct voxtree_n {
    union {
        struct voxtree_n *children;
        cdss_integer_t data;
    } u;
    unsigned isleaf;
};

struct voxtree {
    unsigned depth;
    unsigned dim;
    unsigned child_count;
    cdss_alloc_t allocator;
    struct voxtree_n node;
};

size_t
voxtree_get_alloc_size(unsigned dimensions)
{
    return sizeof(struct voxtree_n)*(1 << dimensions);
}

voxtree_t *
voxtree_create(unsigned dimensions, unsigned depth, const cdss_alloc_t *allocator, cdss_integer_t inital)
{
    if(allocator && allocator->type == ALLOC_SYM)
        if(allocator->u.symmetric.size < sizeof(struct voxtree_n)*(1 << dimensions))
            return 0;//some idiot gave us a symmetric allocator that isn't big enough

    struct voxtree *ret;
    ret = (voxtree_t *)malloc(sizeof(struct voxtree) + sizeof(unsigned long)*dimensions);
    ret->depth = depth;
    ret->dim = dimensions;
    ret->child_count = 1 << dimensions;

    if(allocator)
        ret->allocator = *allocator;
    else
        ret->allocator = ALLOC_STDLIB;

    ret->node.isleaf = 1;
    ret->node.u.data = inital;

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
                cdss_free(&tree->allocator, node->u.children);

                node->isleaf = 1;
                node_stack_index--;
            }
        }
    }

    free(tree);
}

cdss_integer_t
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
voxtree_set(voxtree_t *tree, const unsigned long pos[], cdss_integer_t data)
{
    unsigned i;
    unsigned long width = 1 << tree->depth;

    for(i=0; i<tree->dim; i++)
        assert(pos[i] < width);

    cdss_alloc_t allocator = tree->allocator;
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

    // step 2: return if already set correctly
    // if(node->u.data == data)
    if (CDSS_INT_CMP_ALL(node->u.data, data))
        return;

    //step 3: create any new nodes (if necessicary)
    const unsigned d = tree->depth;
    while(node_stack_index < d)
    {
        struct voxtree_n tmp = *node;
        node->isleaf = 0;

        node->u.children = (struct voxtree_n *) cdss_malloc(&allocator, sizeof(struct voxtree_n)*tree->child_count);

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

        cdss_integer_t tmp = node->u.children[0].u.data;

        unsigned i;
        for(i=1; i<tree->child_count; i++)
        {
            if(!node->u.children[i].isleaf)
                return;

            if (!CDSS_INT_CMP_ALL(tmp, node->u.children[i].u.data)) return;
        }

        cdss_free(&tree->allocator, node->u.children);

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

// TODO: optimimize
void
voxtree_iterate_nodes(voxtree_t *tree, voxtree_node_cb cb, int leaves_only)
{
    struct stack {
        struct voxtree_n *node;
        long              aabb[];
    };

    const size_t stack_elem_size = sizeof(struct stack) + sizeof(long) * tree->dim * 2;

    /*
     * Super wonky malloc-like code on the stack
     */
    unsigned char stack_data[stack_elem_size * (tree->depth * tree->child_count +1)];
    //CANT DO THIS HERE, [] are confusing!
    //struct stack *stack = (struct stack *)stack_data;

    unsigned char node_data[stack_elem_size];
    struct stack *node = (struct stack *)node_data;

    int stack_index = 1;
    ((struct stack *)stack_data)->node   = &tree->node;

    for (int i = 0; i < tree->dim; i++)
    {
        ((struct stack *)stack_data)->aabb[i]             = 0;
        ((struct stack *)stack_data)->aabb[i + tree->dim] = 2 << tree->depth;
    }

    while (stack_index != 0)
    {
        memcpy(node, stack_data + --stack_index*stack_elem_size, stack_elem_size);

        if (!node->node->isleaf)
        {
            for (int i = 0; i < tree->child_count; i++)
            {
                unsigned char tmp_data[stack_elem_size];
                struct stack *tmp = (struct stack *)tmp_data;
                tmp->node = &node->node->u.children[i];

                // TODO: verify aabb code
                /*
                 * Shrink the aabb to the correct partition by
                 * averaging all points twards the correct corner
                 */
                for (int j = 0; j < tree->dim; j++)
                {
                    if ((i >> (tree->dim - j)) & 1)
                        tmp->aabb[j] = (tmp->aabb[j] + tmp->aabb[j + tree->dim]) / 2;
                    else
                        tmp->aabb[j + tree->dim] = (tmp->aabb[j] + tmp->aabb[j + tree->dim]) / 2;
                }

                memcpy(stack_data + stack_index++*stack_elem_size, tmp, stack_elem_size);
            }
        }

        if (!(leaves_only && !node->node->isleaf))
        {
            unsigned char region_mem[sizeof(voxtree_region_t) + sizeof(long) * tree->dim * 2];

            voxtree_region_t *r = (voxtree_region_t *)region_mem;

            if (node->node->isleaf) r->data = node->node->u.data;
            for (int i = 0; i < tree->dim * 2; i++) r->bounds[i] = node->aabb[i];

            cb(node->node->isleaf, r);
        }
    }
}
