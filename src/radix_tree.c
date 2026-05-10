#include "../include/radix_tree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static jp_rnode_t* rnode_new(jp_radix_t* t)
{
    jp_rnode_t* n = (jp_rnode_t *)jp_pool_zalloc(t->pool, sizeof(jp_rnode_t));
    if (n)
    {
        n->flags = RNODE_LEAF;
        t->n_nodes++;
    }
    return n;
}

static uint8_t* pool_bytes(jp_radix_t* t, const uint8_t* src, size_t len)
{
    uint8_t* copy = (uint8_t *)jp_pool_alloc(t->pool, len);
    if (copy) memcpy(copy, src, len);
    return copy;
}

static int rnode_add_child (jp_radix_t* t, jp_rnode_t* n, jp_rnode_t* c)
{
    uint8_t nc = n->n_children;
    jp_rnode_t** arr = (jp_rnode_t **)jp_pool_alloc(t->pool, (size_t)(nc + 1) * sizeof(jp_rnode_t *));
    if (!arr) return -1;

    uint8_t first = c->edge[0];
    uint8_t ins = 0;

    if (nc > 0)
    {
        jp_rnode_t** old = n->u.children;
        for (ins = 0; ins < nc; ins++)
            if (old[ins]->edge[0] > first) break;
        memcpy(arr, old, (size_t)ins * sizeof(jp_rnode_t *));
        arr[ins] = c;
        memcpy(arr + ins + 1, old + ins, (size_t)(nc - ins) * sizeof(jp_rnode_t *));
    }
    else
        arr[0] = c;

    n->u.children = arr;
    n->n_children = nc + 1;
    n->flags &= (uint8_t)~RNODE_LEAF;
    return 0;
}

static jp_rnode_t* rnode_find_child (const jp_rnode_t* n, uint8_t b, int* idx_out)
{
    if (n->n_children == 0) return NULL;
    jp_rnode_t** arr = n->u.children;
    int lo = 0, hi = (int)n->n_children - 1;
    while (lo <= hi)
    {
        int mid = (lo + hi) >> 1;
        uint8_t fb = arr[mid]->edge[0];
        if (fb == b)
        {
            if (idx_out) *idx_out = mid;
            return arr[mid];
        }
        else if (fb < b) lo = mid + 1;
        else hi = mid - 1;
    }
    return NULL;
}

jp_radix_t* jp_radix_new(jp_pool_t* pool)
{
    jp_radix_t* t = (jp_radix_t *)jp_pool_zalloc(pool, sizeof(jp_radix_t));
    if (!t) return NULL;
    t->pool = pool;
    t->root = rnode_new(t);
    return t->root ? t : NULL;
}

int jp_radix_insert (jp_radix_t* t, const uint8_t* key, size_t klen, void* value)
{
    jp_rnode_t* cur = t->root;
    size_t off = 0;

    while (off < klen)
    {
        int child_idx = -1;
        jp_rnode_t* c = rnode_find_child(cur, key[off], &child_idx);

        // 情況 A：無匹配子節點 -> 掛新葉
        if (!c)
        {
            jp_rnode_t* leaf = rnode_new(t);
            if (!leaf) return -1;
            leaf->edge = pool_bytes(t, key + off, klen - off);
            leaf->edge_len = (uint16_t)(klen - off);
            leaf->flags = RNODE_TERMINAL | RNODE_LEAF;
            leaf->value = value;
            leaf->u.leaf_val = value;
            t->n_keys++;
            return rnode_add_child(t, cur, leaf);
        }

        size_t cp = utf8_common_prefix_bytes_n (c->edge, c->edge_len, key + off, klen - off);

        // 情況 B：edge 完全消費 -> 繼續往下
        if (cp == c->edge_len)
        {
            off += cp;
            cur = c;
            continue;
        }

        // 情況 C：部分匹配 -> 分裂節點 
        jp_rnode_t* old_c = rnode_new(t);
        if (!old_c) return -1;
        old_c->edge = c->edge + cp;
        old_c->edge_len = (uint16_t)(c->edge_len - cp);
        old_c->flags = c->flags;
        old_c->value = c->value;
        old_c->n_children = c->n_children;
        old_c->u = c->u;

        c->edge = pool_bytes(t, key + off, cp);
        c->edge_len = (uint16_t)cp;
        c->flags = RNODE_LEAF;
        c->value = NULL;
        c->n_children = 0;
        c->u.children = NULL;

        if (rnode_add_child(t, c, old_c) < 0) return -1;
        if (off + cp == klen)
        {
            c->flags |= RNODE_TERMINAL;
            c->value = value;
            t->n_keys++;
            return 0;
        }
        jp_rnode_t* leaf = rnode_new(t);
        if (!leaf) return -1;
        leaf->edge = pool_bytes(t, key + off + cp, klen - off - cp);
        leaf->edge_len = (uint16_t)(klen - off - cp);
        leaf->flags = RNODE_TERMINAL | RNODE_LEAF;
        leaf->value = value;
        leaf->u.leaf_val = value;
        t->n_keys++;
        return rnode_add_child(t, c, leaf);
    }

    if (!(cur->flags & RNODE_TERMINAL)) t->n_keys++;
    cur->flags |= RNODE_TERMINAL;
    cur->value = value;
    return 0;
}

void* jp_radix_lookup(const jp_radix_t* t, const uint8_t* key, size_t klen)
{
    const jp_rnode_t* cur = t->root;
    size_t off = 0;

    while (off < klen)
    {
        jp_rnode_t* c = rnode_find_child(cur, key[off], NULL);
        if (!c) return NULL;
        size_t el = c->edge_len;
        size_t rem = klen - off;
        if (rem < el || memcmp(c->edge, key + off, el) != 0) return NULL;
        off += el;
        cur = c;
    }
    return (cur->flags & RNODE_TERMINAL) ? cur->value : NULL;
}

typedef struct
{
    uint8_t* path;
    size_t path_len;
    size_t path_cap;
    jp_radix_visit_fn cb;
    void* ud;
    size_t count;
} walk_ctx_t;

static void path_push(walk_ctx_t* ctx, const uint8_t* bytes, size_t n)
{
    size_t need = ctx->path_len + n;
    if (need > ctx->path_cap)
    {
        while (ctx->path_cap < need) ctx->path_cap *= 2;
        ctx->path = (uint8_t *)realloc(ctx->path, ctx->path_cap);
    }
    memcpy(ctx->path + ctx->path_len, bytes, n);
    ctx->path_len += n;
}

// DFS 收集所有以 ctx->path 為前綴的終止節點
static void walk(const jp_rnode_t* n, walk_ctx_t* ctx)
{
    path_push(ctx, n->edge, n->edge_len);
    if (n->flags & RNODE_TERMINAL)
    {
        ctx->cb(ctx->path, ctx->path_len, n->value, ctx->ud);
        ctx->count++;
    }
    if (n->n_children > 0)
    {
        for (uint8_t i = 0; i < n->n_children; i++)
            walk(n->u.children[i], ctx);
    }
    ctx->path_len -= n->edge_len;
}

size_t jp_radix_prefix_search(const jp_radix_t* t, const uint8_t* prefix, size_t plen, jp_radix_visit_fn cb, void* ud)
{
    const jp_rnode_t* cur = t->root;
    size_t off = 0;
    while (off < plen)
    {
        jp_rnode_t* c = rnode_find_child(cur, prefix[off], NULL);
        if (!c) return 0;
        size_t el = c->edge_len;
        size_t rem = plen - off;
        if (rem <= el)
        {
            if (memcmp(c->edge, prefix + off, rem) != 0) return 0;
            walk_ctx_t ctx =
            {
                .path     = (uint8_t *)malloc(plen + 512),
                .path_len = plen,
                .path_cap = plen + 512,
                .cb       = cb, .ud = ud, .count = 0
            };
            memcpy(ctx.path, prefix, plen);
            if (rem == el && (c->flags & RNODE_TERMINAL))
            {
                cb(ctx.path, ctx.path_len, c->value, ud);
                ctx.count++;
            }
            if (c->n_children > 0)
            {
                for (uint8_t i = 0; i < c->n_children; i++)
                    walk(c->u.children[i], &ctx);
            }
            size_t cnt = ctx.count;
            free(ctx.path);
            return cnt;
        }

        if (memcmp(c->edge, prefix + off, el) != 0) return 0;
        off += el;
        cur = c;
    }

    walk_ctx_t ctx = 
    {
        .path     = (uint8_t *)malloc(plen + 512),
        .path_len = plen,
        .path_cap = plen + 512,
        .cb       = cb, .ud = ud, .count = 0
    };
    memcpy(ctx.path, prefix, plen);

    if (cur->flags & RNODE_TERMINAL)
    {
        cb(ctx.path, ctx.path_len, cur->value, ud);
        ctx.count++;
    }
    for (uint8_t i = 0; i < cur->n_children; i++)
        walk(cur->u.children[i], &ctx);

    size_t cnt = ctx.count;
    free(ctx.path);
    return cnt;
}

static void dump_node(const jp_rnode_t* n, int depth)
{
    if (!n) return;
    for (int i = 0; i < depth * 2; i++) putchar(' ');
    putchar('"');
    for (uint16_t i = 0; i < n->edge_len; i++)
    {
        uint8_t b = n->edge[i];
        if (b >= 0x20u && b < 0x7Fu) putchar((char)b);
        else printf("\\x%02X", b);
    }
    putchar('"');
    if (n->flags & RNODE_TERMINAL) printf(" [T] val=%p", n->value);
    printf("  (ch=%u)\n", n->n_children);
    if (n->n_children > 0)
        for (uint8_t i = 0; i < n->n_children; i++)
            dump_node(n->u.children[i], depth + 1);
}

void jp_radix_dump(const jp_radix_t* t)
{
    printf("Radix Tree: %zu keys, %zu nodes\n", t->n_keys, t->n_nodes);
    if (t->root->n_children > 0)
        for (uint8_t i = 0; i < t->root->n_children; i++)
            dump_node(t->root->u.children[i], 0);
}