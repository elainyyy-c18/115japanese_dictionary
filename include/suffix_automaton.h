#ifndef JPDICT_SUFFIX_AUTOMATON_H
#define JPDICT_SUFFIX_AUTOMATON_H

#include "fsa_conjugation.h"
#include <stddef.h>
#include <stdint.h>

#define SAM_ALPHA 27
#define SAM_MAX_STATES 1024
#define SAM_NIL (-1)
#define SAM_MAX_PATTERNS 64

typedef struct 
{
    int len;
    int link;
    int next[SAM_ALPHA];
} sam_state_t;

typedef struct
{
    const char* suffix;
    int len;
    verb_form_t form;
    const char* desc;
} known_suffix_t;

typedef struct jp_sam
{
    sam_state_t st[SAM_MAX_STATES];
    int sz;
    int last;
    known_suffix_t patterns[SAM_MAX_PATTERNS];
    int n_patterns;
} jp_sam_t;

jp_sam_t* jp_sam_new(void);
void jp_sam_destroy(jp_sam_t* sam);
void jp_sam_extend(jp_sam_t* sam, int c);
int jp_sam_is_substring(const jp_sam_t* sam, const char* s);

void jp_sam_register_suffix(jp_sam_t* sam, const char* suffix, verb_form_t form, const char* desc);
void jp_sam_finalize(jp_sam_t* sam);

verb_form_t jp_sam_identify(const jp_sam_t* sam, const char* word, char* matched_suffix_out, size_t out_size, const char** desc_out);
void jp_sam_dump_patterns(const jp_sam_t *sam);

#endif