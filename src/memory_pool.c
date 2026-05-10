#include "../include/pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static inline size_t align_up(size_t n) 
{
    return (n + POOL_ALIGN - 1u) & ~(POOL_ALIGN - 1u);
}
static jp_block_t *block_new(size_t cap) 
{
    jp_block_t *b = (jp_block_t *)malloc(sizeof(jp_block_t));
    if (!b) return NULL;
    b->buf = (uint8_t *)malloc(cap);
    if (!b->buf) { free(b); return NULL; }
    b->cap = cap;
    b->pos = 0;
    b->next = NULL;
    return b;
}

jp_pool_t* jp_pool_new(size_t block_size) 
{
    if (block_size == 0) block_size = POOL_BLOCK_SIZE;
    jp_pool_t* p = (jp_pool_t *)malloc(sizeof(jp_pool_t));
    if (!p) return NULL;

    p->head = block_new(block_size);
    if (!p->head) 
    {
        free(p);
        return NULL;
    }

    p->cur = p->head;
    p->block_size = block_size;
    p->n_allocs = 0;
    p->bytes_alloc = 0;
    p->bytes_total = block_size;
    return p;
}

void* jp_pool_alloc(jp_pool_t* p, size_t n)
{
    if (!p || n == 0) return NULL;
    n = align_up(n);
    if (p->cur->pos + n <= p->cur->cap) 
    {
        void* ptr = p->cur->buf + p->cur->pos;
        p->cur->pos += n;
        p->bytes_alloc += n;
        p->n_allocs++;
        return ptr;
    }

    size_t new_cap = (n > p->block_size) ? n * 2 : p->block_size;
    jp_block_t* b  = block_new(new_cap);
    if (!b) return NULL;

    p->cur->next = b;
    p->cur = b;
    p->bytes_total += new_cap;

    void* ptr = b->buf;
    b->pos = n;
    p->bytes_alloc += n;
    p->n_allocs++;
    return ptr;
}

void* jp_pool_zalloc(jp_pool_t* p, size_t n) 
{
    void* ptr = jp_pool_alloc(p, n);
    if (ptr) memset(ptr, 0, n);
    return ptr;
}

char* jp_pool_strndup(jp_pool_t* p, const char* s, size_t n)
{
    char* copy = (char *)jp_pool_alloc(p, n + 1);
    if (copy) 
    {
        memcpy(copy, s, n);
        copy[n] = '\0';
    }
    return copy;
}

void jp_pool_reset(jp_pool_t* p) 
{
    if (!p) return;
    for (jp_block_t* b = p->head; b; b = b->next) b->pos = 0;
    p->cur = p->head;
    p->n_allocs = 0;
    p->bytes_alloc = 0;
}

void jp_pool_destroy(jp_pool_t* p) 
{
    if (!p) return;
    jp_block_t* b = p->head;
    while (b)
    {
        jp_block_t* nx = b->next;
        free(b->buf);
        free(b);
        b = nx;
    }
    free(p);
}

void jp_pool_stats(const jp_pool_t* p)
{
    if (!p) return;
    double util = (p->bytes_total > 0) ? 100.0 * p->bytes_alloc / p->bytes_total : 0.0;
    printf("[Pool] total_slab=%-8zu  handed_out=%-8zu  util=%5.1f%%  n_allocs=%zu\n", p->bytes_total, p->bytes_alloc, util, p->n_allocs);
}