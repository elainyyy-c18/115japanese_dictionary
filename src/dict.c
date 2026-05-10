#include "../include/dict.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

jp_dict_t* jp_dict_new()
{
    jp_pool_t* pool = jp_pool_new(0);
    if (!pool) return NULL;
    jp_dict_t* d = (jp_dict_t *)jp_pool_zalloc(pool, sizeof(jp_dict_t));
    if (!d)
    {
        jp_pool_destroy(pool);
        return NULL;
    }
    d->pool = pool;
    d->tree = jp_radix_new(pool);
    if (!d->tree)
    {
        jp_pool_destroy(pool);
        return NULL;
    }
    d->n_verbs = 0;
    return d;
}
void jp_dict_destroy(jp_dict_t* d)
{
    if (!d) return;
    jp_pool_destroy(d->pool);
}
const jp_verb_t* jp_dict_add(jp_dict_t* d, const char* dict_form, const char* hiragana, const char* meaning, verb_type_t vtype, uint32_t flags)
{
    if (!d || !dict_form) return NULL;
    if (d->n_verbs >= DICT_MAX_VERBS) return NULL;
    jp_verb_t* v = (jp_verb_t *)jp_pool_zalloc(d->pool, sizeof(jp_verb_t));
    if (!v) return NULL;

    strncpy(v->dict_form, dict_form, sizeof(v->dict_form) - 1);
    if (hiragana) strncpy(v->hiragana, hiragana, sizeof(v->hiragana) - 1);
    if (meaning) strncpy(v->meaning, meaning, sizeof(v->meaning) - 1);
    v->vtype = vtype;
    v->flags = flags;

    if (jp_radix_insert(d->tree, (const uint8_t *)v->dict_form, strlen(v->dict_form), v) != 0)
        return NULL;
    d->verbs[d->n_verbs++] = v;
    return v;
}

const jp_verb_t* jp_dict_lookup(const jp_dict_t* d, const char* dict_form)
{
    if (!d || !dict_form) return NULL;
    return (const jp_verb_t *)jp_radix_lookup(d->tree, (const uint8_t *)dict_form, strlen(dict_form));
}

typedef struct
{
    const jp_verb_t** out;
    size_t max_out;
    size_t count;
} prefix_ctx_t;

static void prefix_visitor(const uint8_t* key, size_t klen, void* value, void* ud)
{
    (void)key;
    (void)klen;
    prefix_ctx_t* c = (prefix_ctx_t *)ud;
    if (c->count < c->max_out) c->out[c->count] = (const jp_verb_t *)value;
    c->count++;
}
size_t jp_dict_prefix_search(const jp_dict_t* d, const char* prefix, const jp_verb_t** out, size_t max_out)
{
    if (!d || !prefix || !out) return 0;
    prefix_ctx_t ctx =
    {
        .out = out,
        .max_out = max_out,
        .count = 0
    };
    jp_radix_prefix_search(d->tree, (const uint8_t *)prefix, strlen(prefix), prefix_visitor, &ctx);
    return ctx.count;
}

void jp_dict_show_verb(const jp_verb_t* v)
{
    if (!v)
    {
        printf("(null verb)\n");
        return;
    }
    printf("  %-15s  %-12s  [%s]  %s%s\n", v->dict_form, v->hiragana[0] ? v->hiragana : "-", jp_vtype_name(v->vtype), v->meaning, (v->flags & VERB_FLAG_IKU_IRREGULAR) ? "  (te形不規則)" : "");
}

void jp_dict_show_conjugations(const jp_verb_t* v)
{
    if (!v) return;
    char buf[128];
    printf("\n┌────────────────────────────────────────────────────────┐\n");
    printf("│  %s  (%s)  │  %s\n",
           v->dict_form, jp_vtype_name(v->vtype), v->meaning);
    printf("├────────────────────────────────────────────────────────┤\n");
    for (verb_form_t f = FORM_DICT; f < FORM_COUNT; f++)
    {
        if (jp_fsa_conjugate(v, f, buf, sizeof(buf)))
            printf("│  %-13s  %s\n", jp_form_name(f), buf);
    }
    printf("└────────────────────────────────────────────────────────┘\n");
}
void jp_dict_show_stats(const jp_dict_t* d)
{
    if (!d) return;
    printf("\n[Dict] verbs=%zu  radix_keys=%zu  radix_nodes=%zu\n", d->n_verbs, d->tree->n_keys, d->tree->n_nodes);
    jp_pool_stats(d->pool);
}

static verb_type_t parse_vtype(const char* s)
{
    if (!strcmp(s, "godan_u")) return VT_GODAN_U;
    if (!strcmp(s, "godan_ku")) return VT_GODAN_KU;
    if (!strcmp(s, "godan_gu")) return VT_GODAN_GU;
    if (!strcmp(s, "godan_su")) return VT_GODAN_SU;
    if (!strcmp(s, "godan_tsu")) return VT_GODAN_TSU;
    if (!strcmp(s, "godan_nu")) return VT_GODAN_NU;
    if (!strcmp(s, "godan_bu")) return VT_GODAN_BU;
    if (!strcmp(s, "godan_mu")) return VT_GODAN_MU;
    if (!strcmp(s, "godan_ru")) return VT_GODAN_RU;
    if (!strcmp(s, "ichidan")) return VT_ICHIDAN;
    if (!strcmp(s, "suru")) return VT_SURU;
    if (!strcmp(s, "kuru")) return VT_KURU;
    return (verb_type_t)-1;
}

int jp_dict_load_csv(jp_dict_t* d, const char* filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) return -1;
    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp))
    {
        char* start = line;
        if (first_line)
        {
            first_line = 0;
            if ((unsigned char)start[0] == 0xEF && (unsigned char)start[1] == 0xBB && (unsigned char)start[2] == 0xBF) {
                start += 3;
            }
        }
        start[strcspn(start, "\r\n")] = '\0';
        if (start[0] == '\0' || start[0] == '#') continue;
 
        char* f[5];
        char* p = start;
        int n = 0;
        while (n < 5 && p)
        {
            f[n++] = p;
            p = strchr(p, '|');
            if (p) *p++ = '\0';
        }
        if (n < 5) continue;

        verb_type_t vt = parse_vtype(f[3]);
        if ((int)vt < 0)
        {
            fprintf(stderr, "[CSV] 未知動詞類型 \"%s\"，略過 %s\n", f[3], f[0]);
            continue;
        }
        uint32_t flags = 0;
        if (!strcmp(f[4], "iku_irregular")) flags = VERB_FLAG_IKU_IRREGULAR;

        if (jp_dict_add(d, f[0], f[1], f[2], vt, flags))
            count++;
    }
    fclose(fp);
    return count;
}