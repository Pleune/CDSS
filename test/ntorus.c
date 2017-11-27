#include "ntorus.h"

#include "cdss/cdss.h"

static size_t cbi = 0;
static void *set_to = (void *)5;

void util_ntorus_cb(ntorus_t *n, void **data)
{
    *data = set_to;
    cbi++;
}

int
test_ntorus_basic(void)
{
    size_t size[3] = {
        10, 10, 10
    };

    size_t move1[3] = {
        4000, 19000, 400
    };

    size_t shift1[3] = {
        1, 1, 1
    };

    size_t it[3];

    size_t out1[2][10][10] = {
        {{9,9,9,9,9,9,9,9,9,9},
         {9,9,9,9,9,9,9,9,9,9},
         {9,9,9,9,9,9,9,9,9,9},
         {9,9,9,9,9,9,9,9,9,9},
         {9,9,9,9,9,9,9,9,9,9},
         {9,9,9,9,9,9,9,9,9,9},
         {9,9,9,9,9,9,9,9,9,9},
         {9,9,9,9,9,9,9,9,9,9},
         {9,9,9,9,9,9,9,9,9,9},
         {9,9,9,9,9,9,9,9,9,9}},

        {{9,9,9,9,9,9,9,9,9,9},
         {9,5,5,5,5,5,5,5,5,5},
         {9,5,5,5,5,5,5,5,5,5},
         {9,5,5,5,5,5,5,5,5,5},
         {9,5,5,5,5,5,5,5,5,5},
         {9,5,5,5,5,5,5,5,5,5},
         {9,5,5,5,5,5,5,5,5,5},
         {9,5,5,5,5,5,5,5,5,5},
         {9,5,5,5,5,5,5,5,5,5},
         {9,5,5,5,5,5,5,5,5,5}}
    };

    ntorus_t *n = ntorus_create(3, size, (void *)1, 0);
    ntorus_callback_in(n, &util_ntorus_cb);
    ntorus_move(n, move1);

    if(cbi != 1000)
    {
        printf("%lu\n", cbi);
        return 1;
    }

    for(it[0]=0; it[0]<size[0]; it[0]++)
    for(it[1]=0; it[1]<size[1]; it[1]++)
    for(it[2]=0; it[2]<size[2]; it[2]++)
    {
        if(*ntorus_at(n, it) != (void *)5)
            return 1;
    }

    cbi = 0;
    set_to = (void *)9;

    ntorus_shift(n, shift1);

    if(cbi != 271)
        return 1;


    for(it[0]=0; it[0]<size[0]; it[0]++)
    for(it[1]=0; it[1]<size[1]; it[1]++)
    for(it[2]=0; it[2]<2; it[2]++)
    {
        if(*ntorus_at(n, it) != (void *)out1[it[2]][it[1]][it[0]])
            return 1;
    }

    ntorus_destroy(n);

    return 0;
}
