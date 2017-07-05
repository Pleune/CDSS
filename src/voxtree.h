#ifndef PLEUNE_VOXTREE_H
#define PLEUNE_VOXTREE_H

#define VOXTREE_INIT(name, depth, alloc_func, free_func, alloc_arg, data_type, inital_data) \
    typedef struct voxtree_##name##_s {                                 \
        unsigned char isleaf;                                           \
        union {                                                         \
            struct voxtree_##name##_s *children;                        \
            data_type data;                                             \
        } u;                                                            \
    } voxtree_##name##_t;                                               \
                                                                        \
    static voxtree_##name##_t *voxtree_##name##_create()                \
    {                                                                   \
        voxtree_##name##_t *ret = alloc_func(alloc_arg);                \
        ret->isleaf = 1;                                                \
        ret->u.data = inital_data;                                      \
        return ret;                                                     \
    }                                                                   \
                                                                        \
    static void voxtree_##name##_destroy(voxtree_##name##_t *tree)      \
    {                                                                   \
        size_t node_stack_index = 1;                                    \
        voxtree_##name##_t *node_stack[depth*8000+1];                   \
        int node_stack_ca[depth*8000+1];                                \
                                                                        \
        node_stack[0] = tree;                                           \
        node_stack_ca[0] = 0;                                           \
                                                                        \
        if(!tree->isleaf)                                               \
        while(node_stack_index > 0)                                     \
        {                                                               \
            voxtree_##name##_t *node = node_stack[--node_stack_index];  \
            int ca = node_stack_ca[node_stack_index];                   \
                                                                        \
            if(ca)                                                      \
            {                                                           \
                free_func(alloc_arg, node->u.children);                 \
                node_stack_index--;                                     \
            } else {                                                    \
                node_stack_ca[node_stack_index] = 1;                    \
                node_stack[node_stack_index++] = node;                  \
                unsigned i;                                             \
                for(i=0; i<8; i++)                                      \
                if(!node->u.children[i].isleaf)                         \
                {                                                       \
                    node_stack[node_stack_index] = &node->u.children[i]; \
                    node_stack_ca[node_stack_index++] = 0;              \
                }                                                       \
            }                                                           \
        }                                                               \
                                                                        \
        free_func(alloc_arg, tree);                                     \
    }                                                                   \
                                                                        \
    static data_type voxtree_##name##_get(voxtree_##name##_t *tree,     \
                                          unsigned long x,              \
                                          unsigned long y,              \
                                          unsigned long z)              \
    {                                                                   \
        while(!tree->isleaf)                                            \
        {                                                               \
            tree = &tree->u.children[(x >= (1 << (depth - 1))) << 0 |   \
                                     (y >= (1 << (depth - 1))) << 1 |   \
                                     (z >= (1 << (depth - 1))) << 2 ];  \
                                                                        \
            x = (x << 1) & (~(1<<depth));                               \
            y = (y << 1) & (~(1<<depth));                               \
            z = (z << 1) & (~(1<<depth));                               \
        }                                                               \
                                                                        \
        return tree->u.data;                                            \
    }                                                                   \
                                                                        \
    static void voxtree_##name##_set(voxtree_##name##_t *tree,          \
                                     unsigned long x,                   \
                                     unsigned long y,                   \
                                     unsigned long z,                   \
                                     data_type data)                    \
    {                                                                   \
        unsigned node_stack_index = 0;                                  \
        voxtree_##name##_t *node_stack[depth];                          \
        voxtree_##name##_t *node = tree;                                \
                                                                        \
        /*step 1: find lowest leaf*/                                    \
        while(!node->isleaf)                                            \
        {                                                               \
            node_stack[node_stack_index++] = node;                      \
            node = &node->u.children[(x >= (1 << (depth - 1))) << 0 |   \
                                     (y >= (1 << (depth - 1))) << 1 |   \
                                     (z >= (1 << (depth - 1))) << 2 ];  \
                                                                        \
            x = (x << 1) & (~(1<<depth));                               \
            y = (y << 1) & (~(1<<depth));                               \
            z = (z << 1) & (~(1<<depth));                               \
        }                                                               \
                                                                        \
        /*step 2: return if already set correctly*/                     \
        if(node->u.data == data)                                        \
            return;                                                     \
                                                                        \
        /*step 3: create any new nodes (if necessicary)*/               \
        while(node_stack_index < depth)                                 \
        {                                                               \
            node->isleaf = 0;                                           \
            voxtree_##name##_t tmp = *node;                             \
            tmp.isleaf = 1;                                             \
            node->u.children = alloc_func(alloc_arg);                   \
                                                                        \
            unsigned i;                                                 \
            for(i=0; i<8; i++)                                          \
                node->u.children[i] = tmp;                              \
                                                                        \
            node_stack[node_stack_index++] = node;                      \
            node = &node->u.children[(x >= (1 << (depth - 1))) << 0 |   \
                                     (y >= (1 << (depth - 1))) << 1 |   \
                                     (z >= (1 << (depth - 1))) << 2 ];  \
                                                                        \
            x = (x << 1) & (~(1<<depth));                               \
            y = (y << 1) & (~(1<<depth));                               \
            z = (z << 1) & (~(1<<depth));                               \
        }                                                               \
                                                                        \
        /*step 4: actually set the data in the new leaf*/               \
        node->u.data = data;                                            \
                                                                        \
        /*step 5: walk back up the tree and compress*/                  \
        while(node_stack_index > 0)                                     \
        {                                                               \
            node = node_stack[--node_stack_index];                      \
            if(!(node->u.children[0].isleaf))                           \
                return;                                                 \
                                                                        \
            data_type tmp = node->u.children[0].u.data;                 \
                                                                        \
            unsigned i;                                                 \
            for(i=1; i<8; i++)                                          \
            {                                                           \
                if(!(node->u.children[i].isleaf))                       \
                    return;                                             \
                if(tmp != node->u.children[i].u.data)                   \
                    return;                                             \
            }                                                           \
                                                                        \
            free_func(alloc_arg, node->u.children);                     \
            node->isleaf = 1;                                           \
            node->u.data = tmp;                                         \
        }                                                               \
    }                                                                   \
                                                                        \
    long long voxtree_##name##_count_nodes(voxtree_##name##_t *tree)    \
    {                                                                   \
        long long nodes = 1;                                            \
        unsigned i;                                                     \
        if(!tree->isleaf)                                               \
            for(i=0; i<8; i++)                                          \
                nodes += voxtree_##name##_count_nodes(&tree->u.children[i]); \
        return nodes;                                                   \
    }

#endif
