#include "cdss/cdss.h"

#include <stdint.h>

uint32_t
util_hmap_hash1(hmap_key_t p)
{
    return p.uint;
}

uint32_t
util_hmap_compare1(hmap_key_t p1, hmap_key_t p2)
{
    return p1.uint == p2.uint;
}

int
test_hmap_basic(void)
{
    hmap_config_t hmapc;
    hmapc.alloc_base = ALLOC_STDLIB;
    hmapc.alloc_node = ALLOC_STDLIB;
    hmapc.buckets = 100;
    hmapc.max_load_factor = 2;
    hmapc.hash_func = &util_hmap_hash1;
    hmapc.compare_func = &util_hmap_compare1;

    hmap_t *hmap = hmap_create(&hmapc);

    unsigned i;

    hmap_insert(hmap, (hmap_keypair_t){{.uint=1}, {.uint=3}});

    for(i=0; i<20; i++)
    {
        uint32_t p = hmap_find(hmap, (hmap_key_t){.uint=i}).uint;

        fflush(stdout);

        if(p)
            printf("Hmap Basic: Found %i:%i\n", i, p);
        else
            printf("Hmap Basic: Not Found %i\n", i);
        fflush(stdout);
    }

    hmap_destroy(hmap);

    return 0;
}
