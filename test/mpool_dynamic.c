#include <time.h>
#include <stdlib.h>

#include "cdss/cdss_all.h"

int
test_mpool_dy_basic(void)
{
    mpool_dy_t *pool = mpool_dy_create(1024*16, sizeof(int), 8);
    int **pointers = malloc(10000*sizeof(int *));
    size_t i;

    //linear test
    for(i=0; i<10000; i++)
        pointers[i] = mpool_dy_alloc(pool);

    for(i=0; i<10000; i++)
        pointers[i][0] = i;

    for(i=0; i<10000; i++)
        if(pointers[i][0] != (int)i) return 1;

    for(i=0; i<10000; i++)
        mpool_dy_free(pool, pointers[i]);

    srand(time(0));

    for(i=0; i<10000; i++)
        pointers[i] = 0;

    for(i=0; i<1000000; i++)
    {
        size_t j = rand()%10000;

        if(pointers[j])
        {
            if(pointers[j][0] != (int)j) return 1;
            mpool_dy_free(pool, pointers[j]);
            pointers[j] = 0;
        } else {
            pointers[j] = mpool_dy_alloc(pool);
            pointers[j][0] = j;
        }
    }

    for(i=0; i<10000; i++)
    {
        if(pointers[i])
        {
            if(pointers[i][0] != (int)i) return 1;
            mpool_dy_free(pool, pointers[i]);
        }
    }

    if(mpool_dy_blocks(pool) != 1)
        return 1;

    mpool_dy_destroy(pool);
    free(pointers);

    return 0;
}

int
main()
{
    return test_mpool_dy_basic();
}
