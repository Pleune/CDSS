#include <time.h>
#include <stdlib.h>

#include "cdss/cdss_all.h"

int
test_mpool_st_basic(void)
{
    mpool_st_t *pool = mpool_st_create(4096*8, sizeof(long long), 8);
    long long **pointers = malloc(10000*sizeof(long long *));
    size_t i;

    //exaust pool
    for(i=0; i<10000; i++)
        pointers[i] = mpool_st_alloc(pool);

    //exausted pool shouldnt alloc
    if(mpool_st_alloc(pool))
        return 1;

    //write zero to make sure we dont lead pointers
    for(i=0; i<10000; i++)
        if(pointers[i]) pointers[i][0] = 0;

    //make sure the writes stuck
    for(i=0; i<10000; i++)
        if(pointers[i])
            if(pointers[i][0] != 0)
                return 1;

    //free everything
    for(i=0; i<10000; i++)
    {
        if(pointers[i])
            mpool_st_free(pool, pointers[i]);
        pointers[i] = 0;
    }

    //jump around like crazy allocing and freeing, making sure values
    //stick and are not corrupted
    srand(time(0));
    for(i=0; i<1000000; i++)
    {
        size_t j = rand()%10000;

        if(pointers[j])
        {
            if(pointers[j][0] != (int)j) return 1;
            mpool_st_free(pool, pointers[j]);
            pointers[j] = 0;
        } else {
            pointers[j] = mpool_st_alloc(pool);
            if(pointers[j]) pointers[j][0] = j;
        }
    }

    //free everything
    for(i=0; i<10000; i++)
    {
        if(pointers[i])
        {
            if(pointers[i][0] != (int)i) return 1;
            mpool_st_free(pool, pointers[i]);
        }
    }

    mpool_st_destroy(pool);
    free(pointers);

    return 0;
}

int
main()
{
    return test_mpool_st_basic();
}
