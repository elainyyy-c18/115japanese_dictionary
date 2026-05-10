#include "../include/fuzzy_search.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static inline int min3(int a, int b, int c)
{
    int m = a < b ? a : b;
    return m < c ? m : c;
}

int jp_levenshtein(const char* a, const char* b, int max_dist)
{
    if (!a || !b) return -1;
    size_t la = strlen(a), lb = strlen(b);
    if (max_dist >= 0)
    {
        size_t diff = (la > lb) ? la - lb : lb - la;
        if ((int)diff > max_dist) return max_dist + 1;
    }
    if (lb > la)
    {
        const char* t = a;
        a = b;
        b = t;
        size_t tn = la;
        la = lb;
        lb = tn;
    }

    int* dp = (int *)malloc((lb + 1) * sizeof(int));
    if (!dp) return -1;

    for (size_t j = 0; j <= lb; j++) dp[j] = (int)j;

    for (size_t i = 1; i <= la; i++)
    {
        int prev_diag = dp[0];
        dp[0] = (int)i;
        int row_min = dp[0];
        for (size_t j = 1; j <= lb; j++)
        {
            int prev_top = dp[j];
            int cost = (a[i-1] == b[j-1]) ? 0 : 1;
            int v = min3(prev_top + 1, dp[j-1] + 1,prev_diag + cost);
            prev_diag = prev_top;
            dp[j] = v;
            if (v < row_min) row_min = v;
        }
        if (max_dist >= 0 && row_min > max_dist)
        {
            free(dp);
            return max_dist + 1;
        }
    }
    int result = dp[lb];
    free(dp);
    return result;
}

static int cmp_match(const void* a, const void* b)
{
    const fuzzy_match_t* ma = (const fuzzy_match_t *)a;
    const fuzzy_match_t* mb = (const fuzzy_match_t *)b;
    if (ma->distance != mb->distance) return ma->distance - mb->distance;
    return strcmp(ma->verb->dict_form, mb->verb->dict_form);
}

size_t jp_fuzzy_search(const jp_dict_t* d, const char* query, int max_dist, fuzzy_match_t* results, size_t max_results)
{
    if (!d || !query || !results || max_results == 0) return 0;
    size_t cap = (d->n_verbs > max_results * 2) ? d->n_verbs : max_results * 2;
    fuzzy_match_t* all = (fuzzy_match_t *)malloc(cap * sizeof(fuzzy_match_t));
    if (!all) return 0;
    size_t n = 0;

    for (size_t i = 0; i < d->n_verbs; i++)
    {
        const jp_verb_t* v = d->verbs[i];
        int dist = jp_levenshtein(query, v->dict_form, max_dist);
        if (dist <= max_dist)
        {
            if (n >= cap)
            {
                cap *= 2;
                fuzzy_match_t* tmp = (fuzzy_match_t *)realloc(all, cap * sizeof(fuzzy_match_t));
                if (!tmp)
                {
                    free(all);
                    return 0;
                }
                all = tmp;
            }
            all[n].verb = v;
            all[n].distance = dist;
            n++;
        }
    }

    qsort(all, n, sizeof(fuzzy_match_t), cmp_match);

    size_t out_n = (n < max_results) ? n : max_results;
    memcpy(results, all, out_n * sizeof(fuzzy_match_t));
    free(all);
    return out_n;
}