#ifndef JPDICT_RADIX_H
#define JPDICT_RADIX_H

#include "memory_pool.h"
#include "utf8.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define RNODE_TERMINAL 0x01u
#define RNODE_LEAF 0x02u

typedef struct jp_rnode
{
    uint8_t* edge;
    uint16_t edge_len;
    uint8_t n_children;
    uint8_t flags;
    union
    {
        struct jp_rnode** children;
        void* leaf_val;
    } u;
    void* value;
} jp_rnode_t;

typedef struct
{
    jp_rnode_t* root;
    jp_pool_t* pool;
    size_t n_keys;
    size_t n_nodes;
} jp_radix_t;

jp_radix_t* jp_radix_new (jp_pool_t* pool);

int jp_radix_insert (jp_radix_t* t, const uint8_t* key, size_t klen, void* value);
void* jp_radix_lookup (const jp_radix_t* t, const uint8_t* key, size_t klen);

typedef void (*jp_radix_visit_fn)(const uint8_t* key, size_t klen, void* value, void* ud);
size_t jp_radix_prefix_search (const jp_radix_t* t, const uint8_t* prefix, size_t plen, jp_radix_visit_fn cb, void* ud);
void jp_radix_dump(const jp_radix_t* t);

#endif