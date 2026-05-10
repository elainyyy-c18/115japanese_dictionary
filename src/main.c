#ifdef _WIN32
#include <windows.h>
#endif
#include "../include/dict.h"
#include "../include/fuzzy_search.h"
#include "../include/suffix_automaton.h"
#include <stdio.h>
#include <string.h>
static void section(const char* title)
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("════════════════════════════════════════════════════════════════════\n");
}

static void load_verbs(jp_dict_t* d)
{
    // 五段動詞
    jp_dict_add(d, "kau",      "かう",     "to buy",            VT_GODAN_U,   0);
    jp_dict_add(d, "au",       "あう",     "to meet",           VT_GODAN_U,   0);
    jp_dict_add(d, "iu",       "いう",     "to say",            VT_GODAN_U,   0);
    jp_dict_add(d, "kaku",     "かく",     "to write",          VT_GODAN_KU,  0);
    jp_dict_add(d, "kiku",     "きく",     "to listen",         VT_GODAN_KU,  0);
    jp_dict_add(d, "iku",      "いく",     "to go",             VT_GODAN_KU,  VERB_FLAG_IKU_IRREGULAR);
    jp_dict_add(d, "oyogu",    "およぐ",   "to swim",           VT_GODAN_GU,  0);
    jp_dict_add(d, "isogu",    "いそぐ",   "to hurry",          VT_GODAN_GU,  0);
    jp_dict_add(d, "hanasu",   "はなす",   "to speak",          VT_GODAN_SU,  0);
    jp_dict_add(d, "kasu",     "かす",     "to lend",           VT_GODAN_SU,  0);
    jp_dict_add(d, "matsu",    "まつ",     "to wait",           VT_GODAN_TSU, 0);
    jp_dict_add(d, "tatsu",    "たつ",     "to stand",          VT_GODAN_TSU, 0);
    jp_dict_add(d, "shinu",    "しぬ",     "to die",            VT_GODAN_NU,  0);
    jp_dict_add(d, "asobu",    "あそぶ",   "to play",           VT_GODAN_BU,  0);
    jp_dict_add(d, "yobu",     "よぶ",     "to call",           VT_GODAN_BU,  0);
    jp_dict_add(d, "nomu",     "のむ",     "to drink",          VT_GODAN_MU,  0);
    jp_dict_add(d, "yomu",     "よむ",     "to read",           VT_GODAN_MU,  0);
    jp_dict_add(d, "kaeru",    "かえる",   "to return home",    VT_GODAN_RU,  0);
    jp_dict_add(d, "toru",     "とる",     "to take",           VT_GODAN_RU,  0);

    // 一段動詞
    jp_dict_add(d, "taberu",   "たべる",   "to eat",            VT_ICHIDAN,   0);
    jp_dict_add(d, "miru",     "みる",     "to see",            VT_ICHIDAN,   0);
    jp_dict_add(d, "neru",     "ねる",     "to sleep",          VT_ICHIDAN,   0);
    jp_dict_add(d, "okiru",    "おきる",   "to wake up",        VT_ICHIDAN,   0);

    // 不規則動詞
    jp_dict_add(d, "suru",         "する",         "to do",         VT_SURU,  0);
    jp_dict_add(d, "benkyousuru",  "べんきょうする","to study",      VT_SURU,  0);
    jp_dict_add(d, "kuru",         "くる",         "to come",       VT_KURU,  0);
}

static void register_suffixes(jp_sam_t* sam)
{
    // 丁寧語
    jp_sam_register_suffix(sam, "mashita",     FORM_TA,         "丁寧過去");
    jp_sam_register_suffix(sam, "masen",       FORM_MASEN,      "丁寧否定");
    jp_sam_register_suffix(sam, "masu",        FORM_MASU,       "丁寧非過去");

    // 否定
    jp_sam_register_suffix(sam, "nakatta",     FORM_NAKATTA,    "普通否定過去");
    jp_sam_register_suffix(sam, "nai",         FORM_NAI,        "普通否定");

    // 完了、後悔 (てしまう)
    jp_sam_register_suffix(sam, "teshimatta",  FORM_TESHIMAU,   "後悔/完了 (一段)");
    jp_sam_register_suffix(sam, "tteshimatta", FORM_TESHIMAU,   "後悔/完了 (う/つ/る五段)");
    jp_sam_register_suffix(sam, "iteshimatta", FORM_TESHIMAU,   "後悔/完了 (く五段)");
    jp_sam_register_suffix(sam, "ideshimatta", FORM_TESHIMAU,   "後悔/完了 (ぐ五段)");
    jp_sam_register_suffix(sam, "shiteshimatta",FORM_TESHIMAU,  "後悔/完了 (す五段)");
    jp_sam_register_suffix(sam, "ndeshimatta", FORM_TESHIMAU,   "後悔/完了 (鼻音便)");
    jp_sam_register_suffix(sam, "teshimau",    FORM_TESHIMAU,   "後悔/完了未然 (一段)");

    // 進行、狀態 (ている)
    jp_sam_register_suffix(sam, "teiru",       FORM_TEIRU,      "進行/狀態 (一段)");
    jp_sam_register_suffix(sam, "tteiru",      FORM_TEIRU,      "進行/狀態 (う/つ/る音便)");
    jp_sam_register_suffix(sam, "iteiru",      FORM_TEIRU,      "進行/狀態 (く音便)");
    jp_sam_register_suffix(sam, "ideiru",      FORM_TEIRU,      "進行/狀態 (ぐ音便)");
    jp_sam_register_suffix(sam, "shiteiru",    FORM_TEIRU,      "進行/狀態 (す音便)");
    jp_sam_register_suffix(sam, "ndeiru",      FORM_TEIRU,      "進行/狀態 (鼻音便)");

    // 可能、passive、使役
    jp_sam_register_suffix(sam, "rareru",      FORM_POTENTIAL,  "可能/受身 (一段)");
    jp_sam_register_suffix(sam, "saseru",      FORM_CAUSATIVE,  "使役");

    // て形、た形 (含全部音便變體)
    jp_sam_register_suffix(sam, "shite",       FORM_TE,         "て形 (す音便)");
    jp_sam_register_suffix(sam, "shita",       FORM_TA,         "た形 (す音便)");
    jp_sam_register_suffix(sam, "tte",         FORM_TE,         "て形 (促音便)");
    jp_sam_register_suffix(sam, "tta",         FORM_TA,         "た形 (促音便)");
    jp_sam_register_suffix(sam, "ite",         FORM_TE,         "て形 (く音便)");
    jp_sam_register_suffix(sam, "ita",         FORM_TA,         "た形 (く音便)");
    jp_sam_register_suffix(sam, "ide",         FORM_TE,         "て形 (ぐ音便)");
    jp_sam_register_suffix(sam, "ida",         FORM_TA,         "た形 (ぐ音便)");
    jp_sam_register_suffix(sam, "nde",         FORM_TE,         "て形 (鼻音便)");
    jp_sam_register_suffix(sam, "nda",         FORM_TA,         "た形 (鼻音便)");
    jp_sam_register_suffix(sam, "te",          FORM_TE,         "て形 (一段)");
    jp_sam_register_suffix(sam, "ta",          FORM_TA,         "た形 (一段)");

    jp_sam_finalize(sam);
}

typedef struct
{
    int n;
    const jp_verb_t* items[16]; // 收集 16 筆顯示
} list_ctx_t;

static void list_visitor(const uint8_t* key, size_t klen, void* value, void* ud)
{
    (void)key;
    (void)klen;
    list_ctx_t* c = (list_ctx_t *)ud;
    if (c->n < 16) c->items[c->n] = (const jp_verb_t *)value;
    c->n++;
}

int main()
{
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    #endif
    setbuf(stdout, NULL);

    // §1 載入詞典
    section("§1  載入詞典");
    jp_dict_t* dict = jp_dict_new();
    load_verbs(dict);
    printf("詞典已載入：%zu 個動詞詞條。\n", dict->n_verbs);

    // §2 Radix Tree 結構
    section("§2  Radix Tree 結構（節點壓縮後）");
    printf("（共享前綴的單字節點被合併成單邊標籤）\n\n");
    jp_radix_dump(dict->tree);

    // §3 精確查詢
    section("§3  精確查詢");
    const char* targets[] = {"taberu", "iku", "benkyousuru", "doesnt_exist"};
    for (size_t i = 0; i < sizeof(targets)/sizeof(*targets); i++)
    {
        const jp_verb_t* v = jp_dict_lookup(dict, targets[i]);
        printf("  lookup(\"%s\") -> %s\n", targets[i], v ? "found" : "NOT FOUND");
        if (v) jp_dict_show_verb(v);
    }

    // §4 前綴搜尋
    section("§4  前綴搜尋");
    const char* prefixes[] = {"ka", "y", "tab", "ben"};
    for (size_t i = 0; i < sizeof(prefixes)/sizeof(*prefixes); i++)
    {
        list_ctx_t ctx = {0};
        size_t n = jp_radix_prefix_search(dict->tree, (const uint8_t *)prefixes[i], strlen(prefixes[i]), list_visitor, &ctx);
        printf("\n  prefix=\"%s\" -> %zu 個結果\n", prefixes[i], n);
        for (int k = 0; k < ctx.n; k++) jp_dict_show_verb(ctx.items[k]);
    }

    // §5 FSA 動詞活用
    section("§5  FSA 活用引擎");

    const char* demo_verbs[] = {"taberu", "kaku", "iku", "matsu", "asobu", "hanasu", "suru", "kuru"};
    for (size_t i = 0; i < sizeof(demo_verbs)/sizeof(*demo_verbs); i++) {
        const jp_verb_t *v = jp_dict_lookup(dict, demo_verbs[i]);
        if (v) jp_dict_show_conjugations(v);
    }

    printf("\n 注意：行く的 て形是 itte 而非 *iite。\n");
    printf("    這透過 VERB_FLAG_IKU_IRREGULAR 在 FSA 中以例外規則處理。\n");

    // §6 Levenshtein 模糊搜尋
    section("§6  Levenshtein 模糊搜尋 (拼字錯誤修正)");

    const char* typos[] = {"tabreu", "iqu", "benkousuru", "okiriu", "tabueru"};
    for (size_t i = 0; i < sizeof(typos)/sizeof(*typos); i++)
    {
        fuzzy_match_t results[5];
        size_t n = jp_fuzzy_search(dict, typos[i], 2, results, 5);
        printf("\n  輸入 \"%s\" (容忍 distance ≤ 2) -> %zu 個建議：\n", typos[i], n);
        for (size_t k = 0; k < n; k++)
        {
            printf("    distance=%d  ", results[k].distance);
            jp_dict_show_verb(results[k].verb);
        }
        if (n == 0) printf("    (無相近結果) \n");
    }

    // §7 Suffix Automaton
    section("§7  Suffix Automaton — 從活用形反推活用類型");
    jp_sam_t* sam = jp_sam_new();
    register_suffixes(sam);
    jp_sam_dump_patterns(sam);
    printf("\n  SAM 內部狀態數：%d (< 2 × 後綴總長) \n\n", sam->sz);
    const char* conjugated[] =
    {
        "tabemashita",      /* 食べました：丁寧過去      */
        "kaitenai",         /* 書いてない：て形＋ない    */
        "asondeshimatta",   /* 遊んでしまった：後悔完了  */
        "ikimasen",         /* 行きません：丁寧否定      */
        "nondeiru",         /* 飲んでいる：進行          */
        "korareru",         /* 来られる：可能・受身      */
        "kawanai",          /* 買わない：普通否定        */
        "hanashita",        /* 話した：た形              */
    };
    for (size_t i = 0; i < sizeof(conjugated)/sizeof(*conjugated); i++)
    {
        char matched[32] = {0};
        const char* desc  = NULL;
        verb_form_t f = jp_sam_identify(sam, conjugated[i], matched, sizeof(matched), &desc);
        if ((int)f >= 0) printf("  \"%s\"  ->  匹配後綴 \"%s\"  (%s, %s)\n", conjugated[i], matched, jp_form_name(f), desc ? desc : "");
        else printf("  \"%s\"  →  無匹配後綴\n", conjugated[i]);
    }

    jp_sam_destroy(sam);

    // §8 統計與資源 
    section("§8  Memory Pool 統計 (單次大塊配置 vs 數百次 malloc)");
    jp_dict_show_stats(dict);

    // §9 互動查詢
    section("§9  互動查詢 (輸入動詞羅馬拼音，輸入 q 結束)");
    printf("提示：\n");
    printf("  - 辭書形      taberu / iku / suru / nomu ...\n");
    printf("  - 拼錯沒關係  tabeuru -> 自動建議 taberu\n");
    printf("  - 活用形      tabemashita -> 識別語尾為「丁寧過去」\n\n");

    char line[128];
    for (;;)
    {
        printf("動詞> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;
        if (!strcmp(line, "q") || !strcmp(line, "quit") || !strcmp(line, "exit"))
        {
            printf("Bye!\n");
            break;
        }

        // 1. 精確查詢：找到就印全部活用
        const jp_verb_t* v = jp_dict_lookup(dict, line);
        if (v)
        {
            jp_dict_show_conjugations(v);
            continue;
        }

        // 2. Suffix Automaton：嘗試識別已活用的語尾
        jp_sam_t* isam = jp_sam_new();
        register_suffixes(isam);
        char imatch[32] = {0};
        const char* idesc = NULL;
        verb_form_t iform = jp_sam_identify(isam, line, imatch, sizeof(imatch), &idesc);
        jp_sam_destroy(isam);
        if ((int)iform >= 0 && imatch[0])
            printf("\n  [SAM] 語尾 \"%s\" -> %s (%s) \n", imatch, jp_form_name(iform), idesc ? idesc : "");

        // 3. Levenshtein 模糊建議
        fuzzy_match_t sug[4];
        size_t ns = jp_fuzzy_search(dict, line, 2, sug, 4);
        if (ns > 0)
        {
            printf("  [模糊] 找不到 \"%s\"，你是否想找：\n", line);
            for (size_t k = 0; k < ns; k++)
                printf("    %-16s  distance=%-2d  %s\n", sug[k].verb->dict_form, sug[k].distance, sug[k].verb->meaning);
        }
        else if ((int)iform < 0) printf("  找不到 \"%s\"，也無相近結果。\n", line);
    }

    jp_dict_destroy(dict);
    printf("\n✓ 詞典與 Memory Pool 已釋放。\n\n");
    return 0;
}