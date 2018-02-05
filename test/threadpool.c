#include <time.h>
#include "cdss/cdss.h"

void
util_tpool_simple(void *v)
{
    static struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 10000000;//10ms
    nanosleep(&t, 0);
}

int
test_tpool_basic(void)
{
    tpool_t *pool = tpool_create(2);

    size_t i;
    for(i=0; i<10; i++)//100ms
        tpool_add(pool, &util_tpool_simple, 0, 0);

    tpool_flush(pool);
    tpool_destroy(pool);

    return 0;
}

int
test_tpool_max_threads(void)
{
    tpool_t *pool = tpool_create(256);

    size_t i;
    for(i=0; i<256*10; i++)//100ms
        tpool_add(pool, &util_tpool_simple, 0, 0);

    tpool_flush(pool);
    tpool_destroy(pool);

    return 0;
}

int
main()
{
    return test_tpool_basic() || test_tpool_max_threads();
}
