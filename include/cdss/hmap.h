#ifndef CDSS_HMAP_H
#define CDSS_HMAP_H

#include <stdlib.h>
#include <stdint.h>
#include "alloc.h"
#include "basetype.h"

typedef cdss_integer_t hmap_key_t;
typedef cdss_integer_t hmap_data_t;

typedef struct {
    hmap_key_t key;
    hmap_data_t data;
} hmap_keypair_t;

typedef struct hmap hmap_t;
typedef uint32_t (*hmap_hash_t)(hmap_key_t);
typedef unsigned (*hmap_compare_t)(hmap_key_t, hmap_key_t);
typedef uint32_t (*hmap_cb_t)(hmap_keypair_t *);

typedef struct {
    alloc_t alloc_base;
    alloc_t alloc_node;
    hmap_hash_t hash_func;
    hmap_compare_t compare_func;
    double max_load_factor;
    size_t buckets;
} hmap_config_t;

hmap_t *hmap_create(const hmap_config_t *);
void hmap_destroy(hmap_t *);
void hmap_for_each(hmap_t *, hmap_cb_t);
void hmap_insert(hmap_t *, hmap_keypair_t);
hmap_data_t hmap_find(hmap_t *, hmap_key_t key);

#endif
