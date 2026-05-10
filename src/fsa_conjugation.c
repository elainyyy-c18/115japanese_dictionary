#include "../include/fsa_conjugation.h"
#include <string.h>
#include <stdio.h>

static const fsa_rule_t DELTA[VT_COUNT][FORM_COUNT] =
{
    [VT_GODAN_U] =
    {
        [FORM_DICT] = {1, "u"},
        [FORM_MASU] = {1, "imasu"},
        [FORM_MASEN] = {1, "imasen"},
        [FORM_TE] = {1, "tte"},
        [FORM_TA] = {1, "tta"},
        [FORM_NAI] = {1, "wanai"},
        [FORM_NAKATTA] = {1, "wanakatta"},
        [FORM_BA] = {1, "eba"},
        [FORM_OU] = {1, "ou"},
        [FORM_IMPERATIVE] = {1, "e"},
        [FORM_POTENTIAL] = {1, "eru"},
        [FORM_PASSIVE] = {1, "wareru"},
        [FORM_CAUSATIVE] = {1, "waseru"},
        [FORM_TEIRU] = {1, "tteiru"},
        [FORM_TESHIMAU] = {1, "tteshimau"},
    },

    [VT_GODAN_KU] =
    {
        [FORM_DICT] = {2, "ku"},
        [FORM_MASU] = {2, "kimasu"},
        [FORM_MASEN] = {2, "kimasen"},
        [FORM_TE] = {2, "ite"},
        [FORM_TA] = {2, "ita"},
        [FORM_NAI] = {2, "kanai"},
        [FORM_NAKATTA] = {2, "kanakatta"},
        [FORM_BA] = {2, "keba"},
        [FORM_OU] = {2, "kou"},
        [FORM_IMPERATIVE] = {2, "ke"},
        [FORM_POTENTIAL] = {2, "keru"},
        [FORM_PASSIVE] = {2, "kareru"},
        [FORM_CAUSATIVE] = {2, "kaseru"},
        [FORM_TEIRU] = {2, "iteiru"},
        [FORM_TESHIMAU] = {2, "iteshimau"},
    },

    [VT_GODAN_GU] =
    {
        [FORM_DICT] = {2, "gu"},
        [FORM_MASU] = {2, "gimasu"},
        [FORM_MASEN] = {2, "gimasen"},
        [FORM_TE] = {2, "ide"},
        [FORM_TA] = {2, "ida"},
        [FORM_NAI] = {2, "ganai"},
        [FORM_NAKATTA] = {2, "ganakatta"},
        [FORM_BA] = {2, "geba"},
        [FORM_OU] = {2, "gou"},
        [FORM_IMPERATIVE] = {2, "ge"},
        [FORM_POTENTIAL] = {2, "geru"},
        [FORM_PASSIVE] = {2, "gareru"},
        [FORM_CAUSATIVE] = {2, "gaseru"},
        [FORM_TEIRU] = {2, "ideiru"},
        [FORM_TESHIMAU] = {2, "ideshimau"},
    },

    [VT_GODAN_SU] =
    {
        [FORM_DICT] = {2, "su"},
        [FORM_MASU] = {2, "shimasu"},
        [FORM_MASEN] = {2, "shimasen"},
        [FORM_TE] = {2, "shite"},
        [FORM_TA] = {2, "shita"},
        [FORM_NAI] = {2, "sanai"},
        [FORM_NAKATTA] = {2, "sanakatta"},
        [FORM_BA] = {2, "seba"},
        [FORM_OU] = {2, "sou"},
        [FORM_IMPERATIVE] = {2, "se"},
        [FORM_POTENTIAL] = {2, "seru"},
        [FORM_PASSIVE] = {2, "sareru"},
        [FORM_CAUSATIVE] = {2, "saseru"},
        [FORM_TEIRU] = {2, "shiteiru"},
        [FORM_TESHIMAU] = {2, "shiteshimau"},
    },

    [VT_GODAN_TSU] =
    {
        [FORM_DICT] = {3, "tsu"},
        [FORM_MASU] = {3, "chimasu"},
        [FORM_MASEN] = {3, "chimasen"},
        [FORM_TE] = {3, "tte"},
        [FORM_TA] = {3, "tta"},
        [FORM_NAI] = {3, "tanai"},
        [FORM_NAKATTA] = {3, "tanakatta"},
        [FORM_BA] = {3, "teba"},
        [FORM_OU] = {3, "tou"},
        [FORM_IMPERATIVE] = {3, "te"},
        [FORM_POTENTIAL] = {3, "teru"},
        [FORM_PASSIVE] = {3, "tareru"},
        [FORM_CAUSATIVE] = {3, "taseru"},
        [FORM_TEIRU] = {3, "tteiru"},
        [FORM_TESHIMAU] = {3, "tteshimau"},
    },

    [VT_GODAN_NU] =
    {
        [FORM_DICT] = {2, "nu"},
        [FORM_MASU] = {2, "nimasu"},
        [FORM_MASEN] = {2, "nimasen"},
        [FORM_TE] = {2, "nde"},
        [FORM_TA] = {2, "nda"},
        [FORM_NAI] = {2, "nanai"},
        [FORM_NAKATTA] = {2, "nanakatta"},
        [FORM_BA] = {2, "neba"},
        [FORM_OU] = {2, "nou"},
        [FORM_IMPERATIVE] = {2, "ne"},
        [FORM_POTENTIAL] = {2, "neru"},
        [FORM_PASSIVE] = {2, "nareru"},
        [FORM_CAUSATIVE] = {2, "naseru"},
        [FORM_TEIRU] = {2, "ndeiru"},
        [FORM_TESHIMAU] = {2, "ndeshimau"},
    },

    [VT_GODAN_BU] =
    {
        [FORM_DICT] = {2, "bu"},
        [FORM_MASU] = {2, "bimasu"},
        [FORM_MASEN] = {2, "bimasen"},
        [FORM_TE] = {2, "nde"},
        [FORM_TA] = {2, "nda"},
        [FORM_NAI] = {2, "banai"},
        [FORM_NAKATTA] = {2, "banakatta"},
        [FORM_BA] = {2, "beba"},
        [FORM_OU] = {2, "bou"},
        [FORM_IMPERATIVE] = {2, "be"},
        [FORM_POTENTIAL] = {2, "beru"},
        [FORM_PASSIVE] = {2, "bareru"},
        [FORM_CAUSATIVE] = {2, "baseru"},
        [FORM_TEIRU] = {2, "ndeiru"},
        [FORM_TESHIMAU] = {2, "ndeshimau"},
    },

    [VT_GODAN_MU] =
    {
        [FORM_DICT] = {2, "mu"},
        [FORM_MASU] = {2, "mimasu"},
        [FORM_MASEN] = {2, "mimasen"},
        [FORM_TE] = {2, "nde"},
        [FORM_TA] = {2, "nda"},
        [FORM_NAI] = {2, "manai"},
        [FORM_NAKATTA] = {2, "manakatta"},
        [FORM_BA] = {2, "meba"},
        [FORM_OU] = {2, "mou"},
        [FORM_IMPERATIVE] = {2, "me"},
        [FORM_POTENTIAL] = {2, "meru"},
        [FORM_PASSIVE] = {2, "mareru"},
        [FORM_CAUSATIVE] = {2, "maseru"},
        [FORM_TEIRU] = {2, "ndeiru"},
        [FORM_TESHIMAU] = {2, "ndeshimau"},
    },

    [VT_GODAN_RU] =
    {
        [FORM_DICT] = {2, "ru"},
        [FORM_MASU] = {2, "rimasu"},
        [FORM_MASEN] = {2, "rimasen"},
        [FORM_TE] = {2, "tte"},
        [FORM_TA] = {2, "tta"},
        [FORM_NAI] = {2, "ranai"},
        [FORM_NAKATTA] = {2, "ranakatta"},
        [FORM_BA] = {2, "reba"},
        [FORM_OU] = {2, "rou"},
        [FORM_IMPERATIVE] = {2, "re"},
        [FORM_POTENTIAL] = {2, "reru"},
        [FORM_PASSIVE] = {2, "rareru"},
        [FORM_CAUSATIVE] = {2, "raseru"},
        [FORM_TEIRU] = {2, "tteiru"},
        [FORM_TESHIMAU] = {2, "tteshimau"},
    },

    [VT_ICHIDAN] =
    {
        [FORM_DICT] = {2, "ru"},
        [FORM_MASU] = {2, "masu"},
        [FORM_MASEN] = {2, "masen"},
        [FORM_TE] = {2, "te"},
        [FORM_TA] = {2, "ta"},
        [FORM_NAI] = {2, "nai"},
        [FORM_NAKATTA] = {2, "nakatta"},
        [FORM_BA] = {2, "reba"},
        [FORM_OU] = {2, "you"},
        [FORM_IMPERATIVE] = {2, "ro"},
        [FORM_POTENTIAL] = {2, "rareru"},
        [FORM_PASSIVE] = {2, "rareru"},  // 一段：可能=受身
        [FORM_CAUSATIVE] = {2, "saseru"},
        [FORM_TEIRU] = {2, "teiru"},
        [FORM_TESHIMAU] = {2, "teshimau"},
    },

    [VT_SURU] =
    {
        [FORM_DICT] = {4, "suru"},
        [FORM_MASU] = {4, "shimasu"},
        [FORM_MASEN] = {4, "shimasen"},
        [FORM_TE] = {4, "shite"},
        [FORM_TA] = {4, "shita"},
        [FORM_NAI] = {4, "shinai"},
        [FORM_NAKATTA] = {4, "shinakatta"},
        [FORM_BA] = {4, "sureba"},
        [FORM_OU] = {4, "shiyou"},
        [FORM_IMPERATIVE] = {4, "shiro"},
        [FORM_POTENTIAL] = {4, "dekiru"},
        [FORM_PASSIVE] = {4, "sareru"},
        [FORM_CAUSATIVE] = {4, "saseru"},
        [FORM_TEIRU] = {4, "shiteiru"},
        [FORM_TESHIMAU] = {4, "shiteshimau"},
    },

    [VT_KURU] =
    {
        [FORM_DICT] = {4, "kuru"},
        [FORM_MASU] = {4, "kimasu"},
        [FORM_MASEN] = {4, "kimasen"},
        [FORM_TE] = {4, "kite"},
        [FORM_TA] = {4, "kita"},
        [FORM_NAI] = {4, "konai"},
        [FORM_NAKATTA] = {4, "konakatta"},
        [FORM_BA] = {4, "kureba"},
        [FORM_OU] = {4, "koyou"},
        [FORM_IMPERATIVE] = {4, "koi"},
        [FORM_POTENTIAL] = {4, "korareru"},
        [FORM_PASSIVE] = {4, "korareru"},
        [FORM_CAUSATIVE] = {4, "kosaseru"},
        [FORM_TEIRU] = {4, "kiteiru"},
        [FORM_TESHIMAU] = {4, "kiteshimau"},
    },
};

static const fsa_rule_t IKU_SPECIAL[FORM_COUNT] =
{
    [FORM_TE] = {2, "tte"},
    [FORM_TA] = {2, "tta"},
    [FORM_TEIRU] = {2, "tteiru"},
    [FORM_TESHIMAU] = {2, "tteshimau"},
};

char *jp_fsa_conjugate (const jp_verb_t* verb, verb_form_t form, char* out, size_t out_size)
{
    if (!verb || (unsigned)form >= FORM_COUNT || (unsigned)verb->vtype >= VT_COUNT)
        return NULL;
    const fsa_rule_t* rule = &DELTA[verb->vtype][form];
    if ((verb->flags & VERB_FLAG_IKU_IRREGULAR) && (form == FORM_TE || form == FORM_TA || form == FORM_TEIRU || form == FORM_TESHIMAU))
        rule = &IKU_SPECIAL[form];

    size_t dlen = strlen(verb->dict_form);
    if (dlen < rule->strip) return NULL;

    size_t stem_len = dlen - rule->strip;
    size_t suffix_len = strlen(rule->suffix);

    if (stem_len + suffix_len + 1 > out_size) return NULL;

    memcpy(out, verb->dict_form, stem_len);
    memcpy(out + stem_len, rule->suffix, suffix_len);
    out[stem_len + suffix_len] = '\0';

    return out;
}

static const char* FORM_NAMES[FORM_COUNT] =
{
    "辞書形",
    "ます形",
    "ません形",
    "て形",
    "た形",
    "ない形",
    "なかった形",
    "ば形",
    "意志形",
    "命令形",
    "可能形",
    "受身形",
    "使役形",
    "ている形",
    "てしまう形",
};

static const char* VTYPE_NAMES[VT_COUNT] =
{
    "五段-う", "五段-く", "五段-ぐ", "五段-す", "五段-つ", "五段-ぬ", "五段-ぶ", "五段-む", "五段-る", "一段",    "サ変",    "カ変",
};

const char* jp_form_name (verb_form_t form)
{
    return (unsigned)form < FORM_COUNT ? FORM_NAMES[form] : "?";
}
const char* jp_vtype_name(verb_type_t vt)
{
    return (unsigned)vt   < VT_COUNT   ? VTYPE_NAMES[vt]  : "?";
}