#include <string.h>
#include "cdss/cdss_all.h"

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

    if (!ringbuff_empty(buff)) return 1;

    //test overflow stability
    for(int i=0; i<100; i++)
        ringbuff_put(buff, origional, 27);

    for(int i=0; i<100; i++)
        ringbuff_remove(buff, 0, 27);

    ringbuff_destroy(buff);

    return 0;
}

int
main()
{
    return test_ringbuff_basic();
}
