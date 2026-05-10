#ifndef JPDICT_DICT_H
#define JPDICT_DICT_H

#include "memory_pool.h"
#include "radix_tree.h"
#include "fsa_conjugation.h"
#include <stddef.h>

#define DICT_MAX_VERBS 512

typedef struct jp_dict
{
    jp_pool_t* pool;
    jp_radix_t* tree;
    jp_verb_t* verbs[DICT_MAX_VERBS];
    size_t n_verbs;
} jp_dict_t;

jp_dict_t* jp_dict_new(void);
void jp_dict_destroy(jp_dict_t* d);
const jp_verb_t* jp_dict_add(jp_dict_t* d, const char* dict_form, const char* hiragana, const char* meaning, verb_type_t vtype, uint32_t flags);
const jp_verb_t* jp_dict_lookup(const jp_dict_t* d, const char* dict_form);
size_t jp_dict_prefix_search(const jp_dict_t* d, const char* prefix, const jp_verb_t** out, size_t max_out);

void jp_dict_show_verb(const jp_verb_t* verb);
void jp_dict_show_conjugations(const jp_verb_t* verb);
void jp_dict_show_stats(const jp_dict_t* d);

#endif