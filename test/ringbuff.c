#include "ringbuff.h"

#include <string.h>
#include "pleune.h"

int test_ringbuff_basic(void)
{
    ringbuff_t *buff = ringbuff_create(100);
    const char *origional = "abcdefghijklmnopqrstuvwxyz";
    size_t i;

    //test to make sure data is read/written intact
    for(i=0; i<100000; i++)
    {
        char returned[27];
        ringbuff_put(buff, origional, 27);
        ringbuff_put(buff, origional, 27);

        ringbuff_remove(buff, returned, 27);
        if(strncmp(returned, origional, 27) != 0)
            return 1;

        ringbuff_remove(buff, returned, 27);
        if(strncmp(returned, origional, 27) != 0)
            return 1;
    }
    return ringbuff_empty(buff) ? 0 : 1;

    //test overflow stability
    for(int i=0; i<100; i++)
        ringbuff_put(buff, origional, 27);

    for(int i=0; i<100; i++)
        ringbuff_remove(buff, 0, 27);

    return 0;
}