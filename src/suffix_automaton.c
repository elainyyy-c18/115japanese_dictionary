#include "../include/suffix_automaton.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static inline int sam_idx(int c)
{
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c == '$') return 26;
    return -1;
}

static int sam_new_state(jp_sam_t* sam)
{
    int id = sam->sz++;
    if (id >= SAM_MAX_STATES)
    {
        fprintf(stderr, "SAM: state limit exceeded\n");
        exit(1);
    }
    sam->st[id].len = 0;
    sam->st[id].link = SAM_NIL;
    for (int i = 0; i < SAM_ALPHA; i++) sam->st[id].next[i] = SAM_NIL;
    return id;
}

jp_sam_t* jp_sam_new(void)
{
    jp_sam_t* sam = (jp_sam_t *)calloc(1, sizeof(jp_sam_t));
    if (!sam) return NULL;
    sam->sz = 0;
    int root = sam_new_state(sam);
    sam->st[root].len = 0;
    sam->st[root].link = SAM_NIL;
    sam->last = 0;
    return sam;
}

void jp_sam_destroy(jp_sam_t* sam)
{
    free(sam);
}

void jp_sam_extend(jp_sam_t* sam, int c)
{
    int idx = sam_idx(c);
    if (idx < 0) return;
    int cur = sam_new_state(sam);
    sam->st[cur].len = sam->st[sam->last].len + 1;
    int p = sam->last;
    while (p != SAM_NIL && sam->st[p].next[idx] == SAM_NIL)
    {
        sam->st[p].next[idx] = cur;
        p = sam->st[p].link;
    }
    if (p == SAM_NIL)
        sam->st[cur].link = 0;
    else
    {
        int q = sam->st[p].next[idx];
        if (sam->st[p].len + 1 == sam->st[q].len)
            sam->st[cur].link = q;
        else
        {
            int clone = sam_new_state(sam);
            sam->st[clone] = sam->st[q];
            sam->st[clone].len = sam->st[p].len + 1;
            while (p != SAM_NIL && sam->st[p].next[idx] == q)
            {
                sam->st[p].next[idx] = clone;
                p = sam->st[p].link;
            }
            sam->st[q].link = clone;
            sam->st[cur].link = clone;
        }
    }
    sam->last = cur;
}

int jp_sam_is_substring(const jp_sam_t* sam, const char* s)
{
    int state = 0;
    for (; *s; s++)
    {
        int idx = sam_idx(*s);
        if (idx < 0) return 0;
        if (sam->st[state].next[idx] == SAM_NIL) return 0;
        state = sam->st[state].next[idx];
    }
    return 1;
}

void jp_sam_register_suffix(jp_sam_t* sam, const char* suffix, verb_form_t form, const char* desc)
{
    if (sam->n_patterns >= SAM_MAX_PATTERNS) return;
    int n = sam->n_patterns++;
    sam->patterns[n].suffix = suffix;
    sam->patterns[n].len = (int)strlen(suffix);
    sam->patterns[n].form = form;
    sam->patterns[n].desc = desc;
}

void jp_sam_finalize(jp_sam_t* sam)
{
    sam->last = 0;
    for (int i = 0; i < sam->n_patterns; i++)
    {
        const char* s = sam->patterns[i].suffix;
        int len = sam->patterns[i].len;
        for (int j = len - 1; j >= 0; j--) jp_sam_extend(sam, s[j]);
        jp_sam_extend(sam, '$');
    }
}

verb_form_t jp_sam_identify(const jp_sam_t* sam, const char* word, char* matched_suffix_out, size_t out_size, const char** desc_out)
{
    if (!sam || !word) return (verb_form_t) - 1;
    int wlen = (int)strlen(word);
    int state = 0;
    int sep_idx = sam_idx('$');

    int candidates[SAM_MAX_PATTERNS], n_cand = 0;
    for (int k = 1; k <= wlen && n_cand < SAM_MAX_PATTERNS; k++)
    {
        int c = word[wlen - k];
        int idx = sam_idx(c);
        if (idx < 0) break;
        if (sam->st[state].next[idx] == SAM_NIL) break;
        state = sam->st[state].next[idx];

        if (sam->st[state].next[sep_idx] != SAM_NIL)
            candidates[n_cand++] = k;
    }

    for (int i = n_cand - 1; i >= 0; i--)
    {
        int k = candidates[i];
        for (int p = 0; p < sam->n_patterns; p++){
            if (sam->patterns[p].len == k && memcmp(word + wlen - k, sam->patterns[p].suffix, (size_t)k) == 0)
            {
                if (matched_suffix_out && out_size > (size_t)k)
                {
                    memcpy(matched_suffix_out, sam->patterns[p].suffix, (size_t)k);
                    matched_suffix_out[k] = '\0';
                }
                if (desc_out) *desc_out = sam->patterns[p].desc;
                return sam->patterns[p].form;
            }
        }
    }
    if (matched_suffix_out && out_size > 0) matched_suffix_out[0] = '\0';
    if (desc_out) *desc_out = NULL;
    return (verb_form_t)-1;
}

void jp_sam_dump_patterns(const jp_sam_t* sam)
{
    printf("[SAM] %d states, %d patterns\n", sam->sz, sam->n_patterns);
    for (int i = 0; i < sam->n_patterns; i++)
    {
        printf("  %2d: %-12s %-12s  %s\n", i, sam->patterns[i].suffix, jp_form_name(sam->patterns[i].form), sam->patterns[i].desc ? sam->patterns[i].desc : "");
    }
}