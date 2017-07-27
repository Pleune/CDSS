#include "voxtree.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "pleune.h"

int
test_voxtree_basic(void)
{
    voxtree_t *tree = voxtree_create(5, 0, 0, 0);//32x32x32

    void *i = (void *)100;
    voxtree_set(tree, 10, 20, 10, i);
    i = voxtree_get(tree, 10, 20, 10);

    if(i != (void *)100)
        return 1;

    voxtree_set(tree, 10, 20, 10, 0);

    if(voxtree_count_nodes(tree) > 1)
        return 1;

    voxtree_destroy(tree);

    return 0;
}

int
test_voxtree_noise(void)
{
    voxtree_t *tree = voxtree_create(5, 0, 0, 0);//32x32x32

    void *data[32][32][32] = {{{0}}};

    srand(time(0));

    long i;
    int x, y, z;
    void *t = 0;
    for(i=0; i<1000000; i++)
    {
        x = rand()%32;
        y = rand()%32;
        z = rand()%32;
        t = t + 1;

        data[x][y][z] = t;
        voxtree_set(tree, x, y, z, t);
    }

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        t = voxtree_get(tree, x, y, z);
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
    mpool_grow_t *pool = mpool_grow_create(4096*16, voxtree_get_alloc_size(), 8);
    voxtree_t *tree = voxtree_create(5, (void *)mpool_grow_alloc, (void *)mpool_grow_free, pool);//32x32x32

    void *data[32][32][32] = {{{0}}};

    srand(time(0));

    long i;
    int x, y, z;
    void *t = 0;
    for(i=0; i<1000000; i++)
    {
        x = rand()%32;
        y = rand()%32;
        z = rand()%32;
        t = t + 1;

        data[x][y][z] = t;
        voxtree_set(tree, x, y, z, t);
    }

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        t = voxtree_get(tree, x, y, z);
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

int
test_voxtree_sphere(void)
{
    mpool_grow_t *pool = mpool_grow_create(1024*64, voxtree_get_alloc_size(), 8);
    voxtree_t *tree = voxtree_create(10,
                                     (void *)mpool_grow_alloc,
                                     (void *)mpool_grow_free, pool);//1024x1024x1024

    unsigned x, y, z;

    for(x=0; x<1024; x++)
    for(y=0; y<1024; y++)
    for(z=0; z<1024; z++)
    {
        if(sqrt(x*x + y*y + z*z) < 700)
            voxtree_set(tree, x, y, z, (void *)1);
    }

    voxtree_destroy(tree);
    mpool_grow_destroy(pool);

    return 0;
}