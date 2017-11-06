#include "cdss/hmap.h"

struct hmap_node {
    struct hmap_node *next;
    uint32_t hash;
    hmap_key_t key;
    hmap_data_t data;
};

struct hmap {
    hmap_hash_t hash_func;
    hmap_compare_t compare_func;
    double max_load;
    alloc_t alloc_base, alloc_node;
    size_t b;
    struct hmap_node *buckets[];
};

struct hmap *
hmap_create(const hmap_config_t *c)
{
    struct hmap *ret = 0;

    if(!cdss_alloc_ensure(&c->alloc_base, sizeof(struct hmap)))
        return 0;

    if(!cdss_alloc_ensure(&c->alloc_node, sizeof(struct hmap_node) + sizeof(struct hmap_node *)*c->buckets))
        return 0;

    ret = cdss_calloc(&c->alloc_base, sizeof(struct hmap) + sizeof(struct hmap_node *)*c->buckets);

    if(!ret)
        return 0;

    ret->hash_func = c->hash_func;
    ret->compare_func = c->compare_func;
    ret->max_load = c->max_load_factor;
    ret->alloc_base = c->alloc_base;
    ret->alloc_node = c->alloc_node;
    ret->b = c->buckets;

    return ret;
}

void
hmap_destroy(struct hmap *h)
{
    size_t i;
    struct hmap_node *node, *next;
    for(i = 0; i<h->b; i++)
    for(node = h->buckets[i]; node; node = next)
    {
        next = node->next;
        cdss_free(&h->alloc_node, node);
    }

    cdss_free(&h->alloc_base, h);
}

void
hmap_for_each(struct hmap *h, hmap_cb_t cb)
{
    size_t i;
    struct hmap_node *node;
    for(i = 0; i<h->b; i++)
    for(node = h->buckets[i]; node; node = node->next)
    {
        hmap_keypair_t kp;
        kp.key = node->key;
        kp.data = node->data;
        cb(&kp);
    }
}

void
hmap_insert(struct hmap *h, hmap_keypair_t kp)
{
    uint32_t hash = h->hash_func(kp.key);
    uint32_t bucket = hash % h->b;
    struct hmap_node *new_node = cdss_malloc(&h->alloc_node, sizeof(struct hmap_node));
    new_node->next = h->buckets[bucket];
    new_node->hash = hash;
    new_node->key = kp.key;
    new_node->data = kp.data;
    h->buckets[bucket] = new_node;
}

hmap_data_t
hmap_find(struct hmap *h, hmap_key_t key)
{
    uint32_t hash = h->hash_func(key);
    struct hmap_node *node = h->buckets[hash % h->b];

    while(node)
    {
        if(hash == node->hash && h->compare_func(node->key, key))
            return node->data;

        node = node->next;
    }

    return (hmap_data_t){.pointer = 0};
}

hmap_keypair_t
hmap_remove(struct hmap *h, hmap_key_t key)
{
    uint32_t hash = h->hash_func(key);
    struct hmap_node *node = h->buckets[hash % h->b];
    hmap_keypair_t kp = {{.pointer = 0}, {.pointer = 0}};

    while(node)
    {
        if(hash == node->hash && h->compare_func(node->key, key))
        {
            kp.key = node->key;
            kp.data = node->data;
            return kp;
        }
        node = node->next;
    }

    return kp;
}
