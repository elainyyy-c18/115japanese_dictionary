#ifndef JPDICT_FSA_H
#define JPDICT_FSA_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    VT_GODAN_U = 0,
    VT_GODAN_KU = 1,
    VT_GODAN_GU = 2,
    VT_GODAN_SU = 3,
    VT_GODAN_TSU = 4,
    VT_GODAN_NU = 5,
    VT_GODAN_BU = 6,
    VT_GODAN_MU = 7,
    VT_GODAN_RU = 8,
    VT_ICHIDAN = 9,
    VT_SURU = 10,
    VT_KURU = 11,
    VT_COUNT = 12
} verb_type_t;

typedef enum
{
    FORM_DICT = 0,         // 辞書形
    FORM_MASU = 1,         // ます形
    FORM_MASEN = 2,        // ません形
    FORM_TE = 3,           // て形
    FORM_TA = 4,           // た形
    FORM_NAI = 5,          // ない形
    FORM_NAKATTA = 6,      // なかった形
    FORM_BA = 7,           // ば形
    FORM_OU = 8,           // う/よう形
    FORM_IMPERATIVE = 9,   // 命令形
    FORM_POTENTIAL = 10,   // 可能形
    FORM_PASSIVE = 11,     // 受身形
    FORM_CAUSATIVE = 12,   // 使役形
    FORM_TEIRU = 13,       // ている形
    FORM_TESHIMAU = 14,    // てしまう形
    FORM_COUNT = 15
} verb_form_t;

typedef struct
{
    uint8_t strip;
    const char* suffix;
} fsa_rule_t;

#define VERB_FLAG_IKU_IRREGULAR 0x01u
typedef struct
{
    char dict_form[64];   // 羅馬字辭書形 (Radix Tree 的 key)
    char hiragana[64];    // 平假名表記 (UTF-8)
    char meaning[128];    // 英文釋義
    verb_type_t vtype;    // 動詞類型 (FSA 輸入符號)
    uint32_t flags;
} jp_verb_t;

char* jp_fsa_conjugate (const jp_verb_t* verb, verb_form_t form, char* out, size_t out_size);
const char* jp_form_name(verb_form_t form);
const char* jp_vtype_name(verb_type_t vt);

#endif