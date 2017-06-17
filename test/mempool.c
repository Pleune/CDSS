#include "mempool.h"

#include <time.h>
#include <stdlib.h>

#include "pleune.h"

int
test_mempool_basic(void)
{
    mempool_t *pool = mempool_create(1024, sizeof(int), 8);
    int **pointers = malloc(10000*sizeof(int *));
    size_t i;

    //linear test
    for(i=0; i<10000; i++)
        pointers[i] = mempool_alloc(pool);

    for(i=0; i<10000; i++)
        pointers[i][0] = i;

    for(i=0; i<10000; i++)
        if(pointers[i][0] != (int)i) return 1;

    for(i=0; i<10000; i++)
        mempool_free(pool, pointers[i]);


    //errotic test to scramble free list
    srand(time(0));

    for(i=0; i<10000; i++)
        pointers[i] = 0;

    for(i=0; i<1000000; i++)
    {
        size_t j = rand()%10000;

        if(pointers[j])
        {
            if(pointers[j][0] != (int)j) return 1;
            mempool_free(pool, pointers[j]);
            pointers[j] = 0;
        } else {
            pointers[j] = mempool_alloc(pool);
            pointers[j][0] = j;
        }
    }

    for(i=0; i<10000; i++)
    {
        if(pointers[i])
        {
            if(pointers[i][0] != (int)i) return 1;
            mempool_free(pool, pointers[i]);
        }
    }

    mempool_destroy(pool);
    free(pointers);

    return 0;
}
