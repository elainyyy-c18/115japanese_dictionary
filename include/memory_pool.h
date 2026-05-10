#ifndef JPDICT_POOL_H
#define JPDICT_POOL_H

#include <stddef.h>
#include <stdint.h>

#define POOL_BLOCK_SIZE  (64u * 1024u) 
#define POOL_ALIGN       (sizeof(void *))

typedef struct jp_block 
{
    uint8_t* buf;
    size_t cap;
    size_t pos;
    struct jp_block* next;
} jp_block_t;

typedef struct 
{
    jp_block_t* head;
    jp_block_t* cur;
    size_t block_size;
    size_t n_allocs;
    size_t bytes_alloc;
    size_t bytes_total;
} jp_pool_t;

jp_pool_t* jp_pool_new    (size_t block_size);
void* jp_pool_alloc  (jp_pool_t* p, size_t n);
void* jp_pool_zalloc (jp_pool_t* p, size_t n);
char* jp_pool_strndup(jp_pool_t* p, const char* s, size_t n);
void jp_pool_reset  (jp_pool_t* p);
void jp_pool_destroy(jp_pool_t* p);
void jp_pool_stats  (const jp_pool_t* p);

#endif