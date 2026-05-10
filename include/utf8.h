#ifndef JPDICT_UTF8_H
#define JPDICT_UTF8_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static inline int utf8_char_len(uint8_t lead)
{
    if ((lead & 0x80u) == 0x00u) return 1;
    else if ((lead & 0xE0u) == 0xC0u) return 2;
    else if ((lead & 0xF0u) == 0xE0u) return 3;
    else if ((lead & 0xF8u) == 0xF0u) return 4;
    return 0;
}

uint32_t utf8_decode(const uint8_t** p, const uint8_t* end);
int utf8_encode(uint32_t cp, uint8_t* buf);
size_t utf8_cp_count(const uint8_t* s, size_t byte_len);
size_t utf8_common_prefix_bytes(const uint8_t* a, const uint8_t* b);
size_t utf8_common_prefix_bytes_n(const uint8_t* a, size_t la, const uint8_t* b, size_t lb);

static inline bool utf8_is_hiragana (uint32_t cp)
{
    return cp >= 0x3041u && cp <= 0x309Fu;
} // ぁ–ん

static inline bool utf8_is_katakana (uint32_t cp)
{
    return cp >= 0x30A0u && cp <= 0x30FFu;
} // ァ–ン

static inline bool utf8_is_kanji(uint32_t cp)
{
    return cp >= 0x4E00u && cp <= 0x9FFFu;
}

#define HG_A 0x3042u  // あ
#define HG_I 0x3044u  // い
#define HG_U 0x3046u  // う
#define HG_E 0x3048u  // え
#define HG_O 0x304Au  // お
#define HG_KA 0x304Bu  // か
#define HG_KI 0x304Du  // き
#define HG_KU 0x304Fu
#define HG_KE 0x3051u
#define HG_KO 0x3053u
#define HG_GA 0x304Cu
#define HG_GI 0x304Eu
#define HG_GU 0x3050u
#define HG_GE 0x3052u
#define HG_GO 0x3054u
#define HG_SA 0x3055u
#define HG_SHI 0x3057u
#define HG_SU 0x3059u
#define HG_SE 0x305Bu
#define HG_SO 0x305Du
#define HG_TA 0x305Fu
#define HG_CHI 0x3061u
#define HG_TSU 0x3064u
#define HG_SMTSU 0x3063u // っ (促音)
#define HG_TE 0x3066u
#define HG_TO 0x3068u
#define HG_NA 0x306Au
#define HG_NI 0x306Bu
#define HG_NU 0x306Cu
#define HG_NE 0x306Du
#define HG_NO 0x306Eu
#define HG_BA 0x3070u
#define HG_BI 0x3073u
#define HG_BU 0x3076u
#define HG_BE 0x3079u
#define HG_BO 0x307Cu
#define HG_MA 0x307Eu
#define HG_MI 0x307Fu
#define HG_MU 0x3080u
#define HG_ME 0x3081u
#define HG_MO 0x3082u
#define HG_YA 0x3084u
#define HG_YO 0x3088u
#define HG_RA 0x3089u
#define HG_RI 0x308Au
#define HG_RU 0x308Bu
#define HG_RE 0x308Cu
#define HG_RO 0x308Du
#define HG_WA 0x308Fu
#define HG_WO 0x3092u
#define HG_N  0x3093u
#define HG_DE 0x3067u
#define HG_DA 0x3060u

static inline void hg_to_utf8(uint32_t cp, uint8_t buf[3])
{
    buf[0] = (uint8_t)(0xE0u | (cp >> 12));
    buf[1] = (uint8_t)(0x80u | ((cp >> 6) & 0x3Fu));
    buf[2] = (uint8_t)(0x80u | (cp & 0x3Fu));
}

#endif