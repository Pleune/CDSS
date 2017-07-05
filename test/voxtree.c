#include "voxtree.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "pleune.h"
mpool_dynamic_t *pool;

VOXTREE_INIT(int, 10, mpool_dynamic_alloc, mpool_dynamic_free, pool, int, 0)

int
test_voxtree_basic(void)
{
    pool = mpool_dynamic_create(16*1024, sizeof(voxtree_int_t)*8, 8);

    voxtree_int_t *tree = voxtree_int_create();

    voxtree_int_set(tree, 10, 20, 30, 100);
    if(voxtree_int_get(tree, 10, 20, 30) != 100)
        return 1;

    voxtree_int_set(tree, 10, 20, 30, 0);
    printf("nodes: %lli\r\n", voxtree_int_count_nodes(tree));
    if(voxtree_int_count_nodes(tree) != 1)
        return 1;

    voxtree_int_destroy(tree);
    mpool_dynamic_destroy(pool);

    return 0;
}

int
test_voxtree_noise(void)
{
    pool = mpool_dynamic_create(16*1024, sizeof(voxtree_int_t)*8, 8);
    voxtree_int_t *tree = voxtree_int_create();

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
        voxtree_int_set(tree, x, y, z, t);
    }

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        if(voxtree_int_get(tree, x, y, z) != data[x][y][z])
            return 1;
    }

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
        voxtree_int_set(tree, x, y, z, 0);

    if(voxtree_int_count_nodes(tree) > 1)
        return 1;

    voxtree_int_destroy(tree);
    mpool_dynamic_destroy(pool);

    return 0;
}

int
test_voxtree_sphere(void)
{
    pool = mpool_dynamic_create(16*1024, sizeof(voxtree_int_t)*8, 8);
    voxtree_int_t *tree = voxtree_int_create();

    unsigned x, y, z;
    for(x=0; x<1024; x++)
    for(y=0; y<1024; y++)
    for(z=0; z<100; z++)
    {
        if(sqrt(x*x + y*y + z*z) < 700)
            voxtree_int_set(tree, x, y, z, 1);
    }

    //voxtree_int_destroy(tree);
    mpool_dynamic_destroy(pool);

    return 0;
}
