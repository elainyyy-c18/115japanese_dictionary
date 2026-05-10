#ifndef JPDICT_FUZZY_SEARCH_H
#define JPDICT_FUZZY_SEARCH_H

#include "dict.h"
#include <stddef.h>

typedef struct
{
    const jp_verb_t* verb;
    int istance;
} fuzzy_match_t;

int jp_levenshtein(const char* a, const char* b, int max_dist);
size_t jp_fuzzy_search(const jp_dict_t* d, const char* query, int max_dist, fuzzy_match_t* results, size_t max_results);

#endif