#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "cdss/cdss_all.h"
#include "cdss/types.h"

static cdss_integer_t cintzero = {.uint = 0};

int
test_voxtree_basic(void)
{
    voxtree_t *tree = voxtree_create(3, 5, 0, cintzero);//32x32x32

    //void *i = (void *)100;
    cdss_integer_t i = {.uint = 100};

    unsigned long pos[3] = {
        10, 20, 10
    };
    voxtree_set(tree, pos, i);
    i = voxtree_get(tree, pos);

    if(i.uint != 100)
        return 1;

    voxtree_set(tree, pos, cintzero);

    if(voxtree_count_nodes(tree) > 1)
        return 1;

    voxtree_destroy(tree);

    return 0;
}

static long noise_cb_int;
static void
noise_cb(int isleaf, const voxtree_region_t *region)
{
    noise_cb_int++;
}

int
test_voxtree_noise(void)
{
    voxtree_t *tree = voxtree_create(3, 5, 0, cintzero);//32x32x32

    unsigned int data[32][32][32] = {{{0}}};

    srand(time(0));

    noise_cb_int = 0;

    long i;
    int x, y, z;
    unsigned int t = 0;
    for(i=0; i<1000000; i++)
    {
        x = rand()%32;
        y = rand()%32;
        z = rand()%32;
        t = t + 1;

        data[x][y][z] = t;

        unsigned long pos[3] = {
            x, y, z
        };
        voxtree_set(tree, pos, (cdss_integer_t){.uint = t});
    }

    voxtree_iterate_nodes(tree, &noise_cb, 0);

    if(voxtree_count_nodes(tree) != noise_cb_int)
        return 1;

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        unsigned long pos[3] = {
            x, y, z
        };
        t = voxtree_get(tree, pos).uint;
        if(t != data[x][y][z])
            return 1;
    }

    t = 0;

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        unsigned long pos[3] = {
            x, y, z
        };
        voxtree_set(tree, pos, (cdss_integer_t){.uint = t});
    }

    if(voxtree_count_nodes(tree) > 1)
        return 1;

    voxtree_destroy(tree);

    return 0;
}

int
test_voxtree_high_dim(void)
{
    voxtree_t *tree = voxtree_create(6, 3, 0, cintzero);//8x8x8x8x8x8

    unsigned int data[8][8][8][8][8][8] = {{{{{{0}}}}}};

    srand(time(0));

    long i;
    unsigned long it[6];
    unsigned int t = 0;
    for(i=0; i<1000000; i++)
    {
        int j;
        for(j=0; j<6; j++)
            it[j] = rand()%8;
        t = t + 1;

        data[it[0]][it[1]][it[2]][it[3]][it[4]][it[5]] = t;
        voxtree_set(tree, it, (cdss_integer_t){.uint = t});
    }

    for(it[0]=0; it[0]<8; it[0]++)
    for(it[1]=0; it[1]<8; it[1]++)
    for(it[2]=0; it[2]<8; it[2]++)
    for(it[3]=0; it[3]<8; it[3]++)
    for(it[4]=0; it[4]<8; it[4]++)
    for(it[5]=0; it[5]<8; it[5]++)
    {
        t = voxtree_get(tree, it).uint;
        if(t != data[it[0]][it[1]][it[2]][it[3]][it[4]][it[5]])
            return 1;
    }

    t = 0;

    for(it[0]=0; it[0]<8; it[0]++)
    for(it[1]=0; it[1]<8; it[1]++)
    for(it[2]=0; it[2]<8; it[2]++)
    for(it[3]=0; it[3]<8; it[3]++)
    for(it[4]=0; it[4]<8; it[4]++)
    for(it[5]=0; it[5]<8; it[5]++)
    {
        voxtree_set(tree, it, (cdss_integer_t){.uint = t});
    }

    if(voxtree_count_nodes(tree) > 1)
        return 1;

    voxtree_destroy(tree);

    return 0;
}

int
test_voxtree_mpool(void)
{
    mpool_grow_t *pool = mpool_grow_create(4096*16, voxtree_get_alloc_size(3), 8);
    alloc_t allocator = mpool_grow_allocator(pool);
    voxtree_t *tree = voxtree_create(3, 5, &allocator, cintzero);//32x32x32

    unsigned int data[32][32][32] = {{{0}}};

    srand(time(0));

    long i;
    int x, y, z;
    unsigned int t = 0;
    for(i=0; i<1000000; i++)
    {
        x = rand()%32;
        y = rand()%32;
        z = rand()%32;
        t = t + 1;

        data[x][y][z] = t;

        unsigned long pos[3] = {
            x, y, z
        };
        voxtree_set(tree, pos, (cdss_integer_t){.uint = t});
    }

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        unsigned long pos[3] = {
            x, y, z
        };
        t = voxtree_get(tree, pos).uint;
        if(t != data[x][y][z])
            return 1;
    }

    t = 0;

    for(x=0; x<32; x++)
    for(y=0; y<32; y++)
    for(z=0; z<32; z++)
    {
        unsigned long pos[3] = {
            x, y, z
        };
        voxtree_set(tree, pos, (cdss_integer_t){.uint = t});
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
    mpool_grow_t *pool = mpool_grow_create(1024*64, voxtree_get_alloc_size(3), 8);
    alloc_t allocator = mpool_grow_allocator(pool);
    voxtree_t *tree = voxtree_create(3, 10, &allocator, cintzero);//1024x1024x1024

    unsigned long pos[3];

#define x pos[0]
#define y pos[1]
#define z pos[2]

    for(x=0; x<1024; x++)
    for(y=0; y<1024; y++)
    for(z=0; z<1024; z++)
    {
        if(sqrt(x*x + y*y + z*z) < 700)
            voxtree_set(tree, pos, (cdss_integer_t){.uint = 1});
    }

    noise_cb_int = 0;
    voxtree_iterate_nodes(tree, &noise_cb, 0);
    if(voxtree_count_nodes(tree) != noise_cb_int)
        return 1;

    voxtree_destroy(tree);
    mpool_grow_destroy(pool);

#undef x
#undef y
#undef z

    return 0;
}

int
main()
{
    return test_voxtree_basic() || test_voxtree_mpool() || test_voxtree_noise() || test_voxtree_high_dim() || test_voxtree_sphere();
}
