# jpdict — 日文動詞詞典與活用引擎

> 一個從零打造的高效能日文動詞活用詞典，以 C 語言探索資訊核心資料結構與演算法。

---

## 目錄

- [jpdict — 日文動詞詞典與活用引擎](#jpdict--日文動詞詞典與活用引擎)
  - [目錄](#目錄)
  - [專案概述](#專案概述)
  - [核心技術一覽](#核心技術一覽)
  - [專案結構](#專案結構)
  - [編譯與執行](#編譯與執行)
  - [模組詳解](#模組詳解)
    - [1. Memory Pool (Arena Allocator)](#1-memory-pool-arena-allocator)
    - [2. UTF-8 位元處理](#2-utf-8-位元處理)
    - [3. Radix Tree（壓縮 Trie）](#3-radix-tree壓縮-trie)
      - [標準 Trie vs Radix Tree](#標準-trie-vs-radix-tree)
      - [節點空間優化（union 技巧）](#節點空間優化union-技巧)
      - [節點分裂（in-place 技巧）](#節點分裂in-place-技巧)
    - [4. FSA — 有限狀態自動機活用](#4-fsa--有限狀態自動機活用)
      - [數學模型](#數學模型)
      - [範例](#範例)
      - [比 if-else 鏈的優勢](#比-if-else-鏈的優勢)
      - [不規則處理](#不規則處理)
    - [5. Levenshtein 編輯距離](#5-levenshtein-編輯距離)
      - [DP 遞推](#dp-遞推)
      - [空間優化（Rolling Array）](#空間優化rolling-array)
      - [早期剪枝](#早期剪枝)
    - [6. Suffix Automaton](#6-suffix-automaton)
      - [經典定義](#經典定義)
      - [線上構造](#線上構造)
      - [本專案的應用](#本專案的應用)
  - [演算法複雜度總表](#演算法複雜度總表)
  - [設計思路](#設計思路)
    - [為什麼是 C？](#為什麼是-c)
    - [取捨](#取捨)

---

## 專案概述

本專案以「日文動詞活用」為應用情境，實際運用 **6 種資料結構與演算法核心技術**：

| # | 模組 | 解決的問題 |
|---|------|-----------|
| 1 | Memory Pool | 高頻配置時的碎片化與 malloc overhead |
| 2 | UTF-8 Bitwise | 多 byte 編碼的字元邊界對齊 |
| 3 | Radix Tree | 共享前綴的字串集合空間優化 |
| 4 | FSA (DFA) | 規則化動詞活用，取代 if-else |
| 5 | Levenshtein DP | 拼字錯誤容忍 |
| 6 | Suffix Automaton | 線性時間後綴匹配 |

---

## 核心技術一覽

```
   ┌──────────────────────────────────────────────────────────────┐
   │                     jp_dict_t (高層 API)                     │
   ├──────────────────────────────────────────────────────────────┤
   │                                                              │
   │   ┌────────────────┐      ┌──────────────────────────────┐   │
   │   │  Radix Tree    │◄─────┤  jp_verb_t  (詞條)           │   │
   │   │  (壓縮 Trie)    │      │   - dict_form (key)          │   │
   │   └───────┬────────┘      │   - hiragana, meaning        │   │
   │           │               │   - vtype, flags             │   │
   │           ▼               └──────────────────────────────┘   │
   │   ┌────────────────┐                                         │
   │   │  Memory Pool   │  ◄── 所有節點 / 邊標籤 / 詞條 共享       │
   │   │  (Arena)       │                                         │
   │   └────────────────┘                                         │
   │                                                              │
   ├──────────────────────────────────────────────────────────────┤
   │   FSA              Fuzzy Search        Suffix Automaton     │
   │   δ(Q,Σ)→Q          Levenshtein DP      Blumer (1985)        │
   └──────────────────────────────────────────────────────────────┘
```

---

## 專案結構

```
jpdict/
├── Makefile
├── README.md
├── .gitignore
├── include/
│   ├── memory_pool.h        記憶體池
│   ├── utf8.h               UTF-8 編碼工具
│   ├── radix_tree.h         Radix Tree
│   ├── fsa_conjugation.h    有限狀態自動機（活用引擎）
│   ├── fuzzy_search.h       Levenshtein 模糊比對
│   ├── suffix_automaton.h   後綴自動機
│   └── dict.h               詞典高層介面
└── src/
    ├── memory_pool.c
    ├── utf8.c
    ├── radix_tree.c
    ├── fsa_conjugation.c
    ├── fuzzy_search.c
    ├── suffix_automaton.c
    ├── dict.c
    └── main.c               示範程式
```

---

## 編譯與執行

【方法一】使用 Makefile (推薦，適用於 Linux / macOS / MinGW)

```bash
make            # 編譯 release 版（-O2）
make run        # 編譯並執行 demo
make debug      # 編譯 debug 版（含 ASan / UBSan）
make clean
```

`make run` 會展示 8 個段落的功能 demo：
1. 載入詞典
2. Radix Tree 結構視覺化
3. 精確查詢
4. 前綴搜尋
5. FSA 活用（含「行く」不規則）
6. Levenshtein 模糊比對
7. Suffix Automaton 後綴識別
8. Memory Pool 統計

【方法二】手動編譯 (適用於無 make 工具之 Windows 環境)
1. 編譯：`gcc src/*.c -Iinclude -o jp_dict -Wall -O2`
2. 執行：`./jp_dict.exe`

---

## 模組詳解

### 1. Memory Pool (Arena Allocator)

**問題**：Radix Tree 數百個節點若都用 `malloc()` 會導致：
- Heap 碎片化
- 每次分配的 metadata overhead（~16 bytes）
- 散落各處的 cache miss

**方案**：預先配置 64 KiB slab，從中線性切割。所有節點共用一個 pool；
詞典銷毀時一次釋放整塊，不需個別 `free()`。

```c
jp_pool_t *pool = jp_pool_new(0);          // 64 KiB 預設
void *p1 = jp_pool_alloc(pool, sizeof(node_t));   // O(1)
void *p2 = jp_pool_alloc(pool, sizeof(node_t));   // O(1)
jp_pool_destroy(pool);                              // O(blocks)
```

對齊技巧：`align_up(n) = (n + ALIGN-1) & ~(ALIGN-1)`，免分支位元運算。

---

### 2. UTF-8 位元處理

日文假名／漢字均為 3-byte UTF-8 序列。標準 `strlen()` 以 byte 計算會在
字元中間截斷。本專案以位元遮罩在 byte stream 層級處理：

```c
static inline int utf8_char_len(uint8_t lead)
{
    if ((lead & 0x80) == 0x00) return 1;   // ASCII
    if ((lead & 0xE0) == 0xC0) return 2;
    if ((lead & 0xF0) == 0xE0) return 3;   // ← 日文在此
    if ((lead & 0xF8) == 0xF0) return 4;
    return 0;
}
```

`utf8_common_prefix_bytes_n()` 在比較完 byte 後，會回退到字元邊界，
確保 Radix Tree 不會在多 byte 字元中間分裂節點。

---

### 3. Radix Tree（壓縮 Trie）

#### 標準 Trie vs Radix Tree

```
Trie (每字元一節點):
   [t]→[a]→[b]→[e]→[r]→[u]    "taberu"
                  └→[m]→[a]→[s]→[u]   "tabemasu"

Radix Tree (邊壓縮):
   ["tabe"]──["ru"]
            └─["masu"]
```

#### 節點空間優化（union 技巧）

```c
typedef struct jp_rnode
{
    uint8_t* edge;        // 邊標籤 bytes
    uint16_t edge_len;
    uint8_t n_children;
    uint8_t flags;
    union
    {
        struct jp_rnode** children;  // 內部節點
        void* leaf_val;  // 葉節點直接存值
    } u;
    void* value;
} jp_rnode_t;
```

**單一 union 在內部 / 葉節點間切換用途**，64-bit 平台佔 32 bytes。

#### 節點分裂（in-place 技巧）

插入新 key 觸發分裂時，**直接修改既有節點**而非更換父節點指標：

```
Before: parent → child[edge="taberu", terminal=A]

Insert "tabemasu" (cp=4="tabe"):
  Step 1: 建立 old_c，繼承 child 原本的尾段
  Step 2: 就地將 child 改為 split node, edge="tabe"
  Step 3: 加 old_c="ru" 作為 child 的子節點
  Step 4: 加新葉 leaf="masu" 作為 child 的子節點

After: parent → child[edge="tabe"]
                  ├─ old_c[edge="ru", terminal=A]
                  └─ leaf [edge="masu", terminal=B]
```

父節點的指標不需更新——`child` 仍位於同一記憶體位址。

---

### 4. FSA — 有限狀態自動機活用

#### 數學模型

轉移函數 **δ : Q × Σ → Q**

- **Q** = 活用形集合（DICT, MASU, TE, TA, NAI, ...）共 15 形
- **Σ** = 動詞類型集合（一段、五段-く、五段-ぐ、する、くる、...）共 12 類型
- **δ** = 二維查找表 `DELTA[verb_type][form]`，共 180 條規則

每條規則 = `{strip_bytes, suffix}`：
- 從辭書形末尾剝離 `strip_bytes` 個 bytes
- 接上 `suffix`

#### 範例

```c
// 五段-く動詞的活用規則
[VT_GODAN_KU] = 
{
    [FORM_MASU] = {2, "kimasu"}, // 書く → 書 + kimasu = 書きます
    [FORM_TE] = {2, "ite"},      // 書く → 書 + ite    = 書いて
    [FORM_NAI] = {2, "kanai"},   // 書く → 書 + kanai  = 書かない
    // ...
};
```

#### 比 if-else 鏈的優勢

- **新增規則只需擴展表格**（O(1) 程式碼變動）
- **表格本身即是規格**，可讀性高，可序列化為資料檔
- **單次活用 O(1) 時間**

#### 不規則處理

「行く（iku）」的 て形是「いって」而非常態的「いいて」。
透過 `VERB_FLAG_IKU_IRREGULAR` 旗標 + 例外規則表覆寫，
保持主表格的純淨。

---

### 5. Levenshtein 編輯距離

#### DP 遞推

```
                 ┌ j                    if i = 0
                 │ i                    if j = 0
   dp[i][j] =    │ dp[i-1][j-1]         if a[i-1] = b[j-1]
                 │ 1 + min(             otherwise
                 │     dp[i-1][j  ],    delete
                 │     dp[i  ][j-1],    insert
                 │     dp[i-1][j-1] )   substitute
                 └
```

#### 空間優化（Rolling Array）

只需保留前一行 → O(min(|a|,|b|)) 空間。

#### 早期剪枝

每完成一行即檢查最小值；若 > `max_dist` 立刻回傳，
平均省下大半計算。

---

### 6. Suffix Automaton

#### 經典定義

由 Blumer et al. (1985) 提出：給定字串 T，SAM(T) 是接受所有 T 的
substring 的最小化 DFA。性質：

| 量 | 上界 |
|---|------|
| 狀態數 | ≤ 2|T| − 1 |
| 邊數 | ≤ 3|T| − 4 |
| 建構時間 | O(|T| · |Σ|) |
| 查詢時間 | O(|P|) |

每個狀態三個欄位：
- `len[v]`：等價類別中最長字串長度
- `link[v]`：suffix link，指向最長真後綴所在等價類別
- `next[v][c]`：δ 轉移

#### 線上構造

```c
void jp_sam_extend(jp_sam_t* sam, int c)
{
    int cur = sam_new_state(sam);
    sam->st[cur].len = sam->st[sam->last].len + 1;
    int p = sam->last;
    while (p != NIL && sam->st[p].next[c] == NIL)
    {
        sam->st[p].next[c] = cur;
        p = sam->st[p].link;
    }

    if (p == NIL) sam->st[cur].link = 0;
    else
    {
        int q = sam->st[p].next[c];
        if (sam->st[p].len + 1 == sam->st[q].len) sam->st[cur].link = q;
        else
        {
            int clone = sam_new_state(sam);
            sam->st[clone] = sam->st[q];
            sam->st[clone].len = sam->st[p].len + 1;
            while (p != NIL && sam->st[p].next[c] == q)
            {
                sam->st[p].next[c] = clone;
                p = sam->st[p].link;
            }
            sam->st[q].link = clone;
            sam->st[cur].link = clone;
        }
    }
    sam->last = cur;
}
```

#### 本專案的應用

**Suffix matching ⇔ Prefix matching**：W 以 P 結尾  ⇔  REV(W) 以 REV(P) 開頭。

故將所有已知活用後綴反轉、以 '$' 分隔串接，建構 SAM(T)：
```
T = "usam" + "$" + "et" + "$" + "atihsam" + "$" + ...
    (REV "masu")    (REV "te")    (REV "mashita")
```

查詢時將輸入字串末尾反轉，從 SAM 出發跟隨字符前進；
每一步檢查是否可走 '$' 轉移——若可，表示走過的字串恰好是某個
反轉後綴的完整匹配。最後以 `memcmp` 嚴格驗證以排除跨界 substring 偽陽性。

---

## 演算法複雜度總表

| 操作 | 複雜度 | 備註 |
|------|--------|------|
| Memory Pool alloc | O(1) | amortised |
| Radix Tree insert | O(\|key\|) | 平均 |
| Radix Tree lookup | O(\|key\|) | |
| Radix Tree prefix search | O(\|prefix\| + #matches × \|key_avg\|) | |
| FSA conjugate | **O(1)** | 查表 + memcpy |
| Levenshtein 距離 | O(\|a\| × \|b\|) | 早期剪枝可大幅優化 |
| SAM 建構 | O(n × \|Σ\|) | n = corpus 長度 |
| SAM 子字串判斷 | O(\|P\|) | |
| SAM 後綴識別 | O(\|word\| × n_patterns) | 含驗證 |

---

## 設計思路

### 為什麼是 C？

- 完全控制記憶體配置（pool 設計）
- 直接操作 byte 流（UTF-8 處理）
- 沒有 hidden allocations，效能可預期

### 取捨

- **Pool 不支援 free**：以重新配置 + 浪費部份 pool 空間換取簡潔
  且不犯 use-after-free
- **Radix Tree 不支援 delete**：日文詞典極少需要刪除詞條
- **SAM 用固定大小 ASCII 表**：日文後綴在內部以羅馬字處理，
  簡化字母表至 27（a-z + '$'）
