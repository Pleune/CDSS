#include "voxtree.h"

#include "stdlib.h"
#include "time.h"

#include "pleune.h"

int
test_voxtree_basic(void)
{
    voxtree_t *tree = voxtree_create(5, 0, 0, 0, sizeof(int));//32x32x32

    int i = 100;
    voxtree_set(tree, 10, 20, 10, &i);
    voxtree_get(tree, 10, 20, 10, &i);

    if(i != 100)
        return 1;

    i = 0;
    voxtree_set(tree, 10, 20, 10, &i);

    if(voxtree_count_nodes(tree) > 1)
        return 1;

    voxtree_destroy(tree);

    return 0;
}

int
test_voxtree_noise(void)
{
    voxtree_t *tree = voxtree_create(5, 0, 0, 0, sizeof(int));//32x32x32

    int data[32][32][32] = {{{0}}};

    srand(time(0));

    long i;
    int x, y, z, t;
    for(i=0; i<1000000; i++)
    {
        x = rand()%32;
        y = rand()%32;
        z = rand()%32;
        t = rand()%1000;

        data[x][y][z] = t;
        voxtree_set(tree, x, y, z, &t);
    }

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        voxtree_get(tree, x, y, z, &t);
        if(t != data[x][y][z])
            return 1;
    }

    t = 0;

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        voxtree_set(tree, x, y, z, &t);
    }

    if(voxtree_count_nodes(tree) > 1)
        return 1;

    voxtree_destroy(tree);

    return 0;
}

int
test_voxtree_mpool(void)
{
    mpool_grow_t *pool = mpool_grow_create(4096*16, voxtree_get_alloc_size(sizeof(int)), 8);
    voxtree_t *tree = voxtree_create(5, (void *)mpool_grow_alloc, (void *)mpool_grow_free, pool, sizeof(int));//32x32x32

    int data[32][32][32] = {{{0}}};

    srand(time(0));

    long i;
    int x, y, z, t;
    for(i=0; i<1000000; i++)
    {
        x = rand()%32;
        y = rand()%32;
        z = rand()%32;
        t = rand()%1000;

        data[x][y][z] = t;
        voxtree_set(tree, x, y, z, &t);
    }

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        voxtree_get(tree, x, y, z, &t);
        if(t != data[x][y][z])
            return 1;
    }

    t = 0;

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        voxtree_set(tree, x, y, z, &t);
    }

    if(voxtree_count_nodes(tree) > 1)
        return 1;

    voxtree_destroy(tree);
    mpool_grow_destroy(pool);

    return 0;
}
