# jpdict — 日文動詞詞典與活用引擎

> 一個從零打造的高效能日文動詞活用詞典，以 C 語言探索資訊核心資料結構與演算法。

---

## 目錄

- [專案概述](#專案概述)
- [核心技術一覽](#核心技術一覽)
- [專案結構](#專案結構)
- [編譯與執行](#編譯與執行)
- [詞典資料庫（verbs.csv）](#詞典資料庫verbscsv)
- [互動查詢模式](#互動查詢模式)
- [模組詳解](#模組詳解)
- [演算法複雜度總表](#演算法複雜度總表)
- [設計思路](#設計思路)

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
   │   ┌────────────────┐      ┌──────────────────────────────┐   │
   │   │  Radix Tree    │◄─────┤  jp_verb_t  (詞條)           │   │
   │   │  (壓縮 Trie)    │      │   - dict_form (key)          │   │
   │   └───────┬────────┘      │   - hiragana, meaning        │   │
   │           ▼               └──────────────────────────────┘   │
   │   ┌────────────────┐                                         │
   │   │  Memory Pool   │  ◄── 所有節點 / 邊標籤 / 詞條 共享       │
   │   └────────────────┘                                         │
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
├── data/
│   └── verbs.csv            動詞資料庫（可直接用 Excel 編輯）
├── include/
│   ├── memory_pool.h        記憶體池
│   ├── utf8.h               UTF-8 編碼工具
│   ├── radix_tree.h         Radix Tree
│   ├── fsa_conjugation.h    有限狀態自動機
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
    └── main.c               示範程式 + 互動查詢
```

---

## 編譯與執行

**方法一：使用 Makefile（推薦，適用 Linux / macOS / MinGW）**

```bash
make            # 編譯 release 版（-O2）
make run        # 編譯並執行
make debug      # 編譯 debug 版（含 ASan / UBSan）
make clean
```

**方法二：手動編譯（適用無 make 的環境）**

```bash
# 編譯
gcc src/*.c -Iinclude -o bin/jp_dict -Wall -O2

# 執行（從專案根目錄執行，才能找到 data/verbs.csv）
./bin/jp_dict
```

> **⚠️ 執行位置很重要**：必須從**專案根目錄**執行，程式才能找到 `data/verbs.csv`。
>
> ```bash
> # ✓ 正確
> cd jpdict
> ./jp_dict
>
> # ✗ 錯誤（找不到 CSV）
> cd jpdict/bin
> ./jp_dict
> ```

> **Windows 亂碼問題**：中文輸出需要 UTF-8 console。執行前先輸入：
> ```bat
> chcp 65001
> ```

執行後依序展示 **9 個段落**：

| § | 內容 |
|---|------|
| §1 | 載入詞典（優先讀取 `data/verbs.csv`） |
| §2 | Radix Tree 結構視覺化（樹狀格式） |
| §3 | 精確查詢 |
| §4 | 前綴搜尋 |
| §5 | FSA 活用引擎（含「行く」不規則） |
| §6 | Levenshtein 模糊比對 |
| §7 | Suffix Automaton 後綴識別 |
| §8 | Memory Pool 統計 |
| §9 | 互動查詢模式 |

---

## 詞典資料庫（verbs.csv）

### 檔案位置

```
jpdict/data/verbs.csv
```

程式啟動時自動從此路徑讀取。找不到檔案則 fallback 至程式碼內的內建詞條。

### 用 Excel 編輯

`verbs.csv` 採用以下設計，確保 Excel 雙擊即可正常開啟：

- **編碼**：UTF-8 with BOM（`EF BB BF`）— Excel 看到 BOM 自動辨識 UTF-8，日文不會亂碼
- **分隔符**：逗號 `,`（標準 CSV）— Excel 自動分欄
- **換行**：CRLF（Windows 格式）

**存檔時**：選「另存新檔」→ 格式選 **CSV UTF-8（逗號分隔）**（有 BOM 的那個選項），
不要選一般的「CSV」，否則會失去 BOM 導致下次開啟日文再次亂碼。

### CSV 格式說明

每一行代表一個動詞，欄位以逗號 `,` 分隔：

```
dict_form,hiragana,meaning,verb_type,flags
```

| 欄位 | 說明 | 範例 |
|------|------|------|
| dict_form | 辭書形（羅馬字） | `taberu` |
| hiragana | 平假名 | `たべる` |
| meaning | 英文釋義 | `to eat` |
| verb_type | 動詞類型（見下表） | `ichidan` |
| flags | 特殊旗標 | `0` 或 `iku_irregular` |

**verb_type 對照表：**

| 填入值 | 對應類型 | 範例動詞 |
|--------|---------|---------|
| `godan_u` | 五段-う | 買う kau |
| `godan_ku` | 五段-く | 書く kaku |
| `godan_gu` | 五段-ぐ | 泳ぐ oyogu |
| `godan_su` | 五段-す | 話す hanasu |
| `godan_tsu` | 五段-つ | 待つ matsu |
| `godan_nu` | 五段-ぬ | 死ぬ shinu |
| `godan_bu` | 五段-ぶ | 遊ぶ asobu |
| `godan_mu` | 五段-む | 飲む nomu |
| `godan_ru` | 五段-る ⚠️ | 帰る kaeru |
| `ichidan` | 一段 | 食べる taberu |
| `suru` | サ変（する） | 勉強する benkyousuru |
| `kuru` | カ変（来る） | 来る kuru |

> ⚠️ **五段-る と 一段の区別**：辭書形以 `-ru` 結尾的動詞，必須手動判斷是五段-る 還是一段。
> 光看羅馬字無法區分，活用方式完全不同：
> ```
> 帰る kaeru  → 五段-る → 帰って  kaette   (godan_ru)
> 食べる taberu → 一段  → 食べて  tabete    (ichidan)
> ```

**範例：**

```csv
# 一段動詞
taberu,たべる,to eat,ichidan,0
miru,みる,to see,ichidan,0

# 五段-く（行く 有不規則 te 形）
kaku,かく,to write,godan_ku,0
iku,いく,to go,godan_ku,iku_irregular

# サ変
benkyousuru,べんきょうする,to study,suru,0
```

### 注解與空行

- `#` 開頭的行為注解，程式讀取時自動略過
- 空行同樣略過，可自由分段排版

### 如何擴充詞彙

在 `data/verbs.csv` 加一行，**重新執行程式即可，不需要重新編譯**：

```csv
kangaeru,かんがえる,to think / to consider,ichidan,0
```

---

## 互動查詢模式

程式跑完 §1–§8 的靜態 demo 後，進入互動模式。

### 1. 辭書形 → 印出全部 15 種活用

```
動詞> taberu
┌────────────────────────────────────────────────────────┐
│  taberu  (一段)  │  to eat
├────────────────────────────────────────────────────────┤
│  辞書形      taberu
│  ます形      tabemasu
│  て形         tabete
│  た形         tabeta
│  ない形      tabenai
│  ...（共 15 種活用形）
└────────────────────────────────────────────────────────┘
```

### 2. 拼字錯誤 → Levenshtein 自動建議

```
動詞> tabeuru
  [模糊] 找不到 "tabeuru"，你是否想找：
    taberu    distance=1   to eat
```

### 3. 已活用的形式 → Suffix Automaton 識別語尾

```
動詞> tabemashita
  [SAM] 語尾 "mashita" → た形（丁寧過去）

動詞> asondeshimatta
  [SAM] 語尾 "ndeshimatta" → てしまう形（後悔/完了 鼻音便）

動詞> nondeiru
  [SAM] 語尾 "ndeiru" → ている形（進行/狀態 鼻音便）
```

輸入 `q` / `quit` / `exit` 離開。

---

## 模組詳解

### 1. Memory Pool (Arena Allocator)

**問題**：Radix Tree 數百個節點若都用 `malloc()` 會導致 Heap 碎片化、
每次分配的 metadata overhead（~16 bytes）、散落各處的 cache miss。

**方案**：預先配置 64 KiB slab，從中線性切割。詞典銷毀時一次釋放，不需個別 `free()`。

```c
jp_pool_t* pool = jp_pool_new(0);                    // 64 KiB 預設
void* p1 = jp_pool_alloc(pool, sizeof(node_t));      // O(1)
jp_pool_destroy(pool);                               // O(blocks)
```

對齊技巧：`align_up(n) = (n + ALIGN-1) & ~(ALIGN-1)`，免分支位元運算。

---

### 2. UTF-8 位元處理

日文假名／漢字均為 3-byte UTF-8 序列。本專案以位元遮罩在 byte stream 層級處理：

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

CSV 解析器額外處理 **UTF-8 BOM**（`EF BB BF`），避免 Excel 存檔後的 BOM 汙染第一個欄位：

```c
if ((unsigned char)start[0] == 0xEF && (unsigned char)start[1] == 0xBB && (unsigned char)start[2] == 0xBF)
    start += 3;
```

---

### 3. Radix Tree（壓縮 Trie）

```
Trie (每字元一節點):
   [t]→[a]→[b]→[e]→[r]→[u]    "taberu"
                  └→[m]→[a]→[s]→[u]   "tabemasu"

Radix Tree (邊壓縮):
   ["tabe"]──["ru"]
            └─["masu"]
```

樹狀輸出範例（`(*)` 終止節點，`[n]` 子節點數）：

```
Radix Tree: 104 keys, 146 nodes
|-- "a"  [7]
|   |-- "geru"  (*)
|   |-- "ruku"  (*)
|   \-- "sobu"  (*)
|-- "k"  [3]
|   |-- "a"  [4]
|   |   |-- "eru"  (*)
|   |   \-- "ku"  (*)
|   \-- "iku"  (*)
```

節點 union 技巧：葉節點與內部節點共用 8 bytes，64-bit 平台整個節點佔 32 bytes。

節點分裂 in-place：修改既有節點而非更換父指標，父節點的指標不需更新。

---

### 4. FSA — 有限狀態自動機活用

轉移函數 **δ : Q × Σ → Q**，二維查找表 `DELTA[verb_type][form]`，
共 12 × 15 = **180 條規則**。每條規則 = `{strip_bytes, suffix}`。

```c
[VT_GODAN_KU] =
{
    [FORM_MASU] = {2, "kimasu"},  // 書く → 書きます
    [FORM_TE]   = {2, "ite"},     // 書く → 書いて
    [FORM_NAI]  = {2, "kanai"},   // 書く → 書かない
};
```

行く（iku）的不規則 te 形透過 `VERB_FLAG_IKU_IRREGULAR` + 例外規則表覆寫。

---

### 5. Levenshtein 編輯距離

```
dp[i][j] = dp[i-1][j-1]              if a[i-1] = b[j-1]
         = 1 + min(dp[i-1][j],        delete
                   dp[i][j-1],        insert
                   dp[i-1][j-1])      substitute
```

空間優化（Rolling Array）：O(min(|a|,|b|)) 空間。早期剪枝：最小值 > max_dist 即停。

---

### 6. Suffix Automaton

由 Blumer et al. (1985) 提出的最小化 DFA，接受字串 T 的所有 substring。

| 量 | 上界 |
|---|------|
| 狀態數 | ≤ 2\|T\| − 1 |
| 建構時間 | O(\|T\| · \|Σ\|) |
| 查詢時間 | O(\|P\|) |

應用：將所有已知後綴反轉並以 `$` 分隔串接建構 SAM(T)。查詢時反轉輸入末尾，
沿 SAM 前進，每步檢查 `$` 轉移；最後以 `memcmp` 驗證排除偽陽性。

已知後綴涵蓋所有音便變體（32 個 pattern）：

| 類型 | 後綴 |
|------|------|
| 丁寧語 | `masu`, `masen`, `mashita` |
| 否定 | `nai`, `nakatta` |
| 完了/後悔（全音便） | `teshimatta`, `tteshimatta`, `ndeshimatta`, `iteshimatta`, `ideshimatta`, `shiteshimatta` |
| 進行（全音便） | `teiru`, `tteiru`, `ndeiru`, `iteiru`, `ideiru`, `shiteiru` |
| 可能・使役 | `rareru`, `saseru` |
| て形・た形（全音便） | `te/ta`, `tte/tta`, `ite/ita`, `nde/nda`, `ide/ida`, `shite/shita` |

---

## 演算法複雜度總表

| 操作 | 複雜度 | 備註 |
|------|--------|------|
| Memory Pool alloc | O(1) | amortised |
| Radix Tree insert | O(\|key\|) | 平均 |
| Radix Tree lookup | O(\|key\|) | |
| Radix Tree prefix search | O(\|prefix\| + #matches × \|key_avg\|) | |
| FSA conjugate | **O(1)** | 查表 + memcpy |
| CSV 載入 n 個詞條 | O(n × \|key\|) | 解析 + 逐條插入 |
| Levenshtein 距離 | O(\|a\| × \|b\|) | 早期剪枝可大幅優化 |
| SAM 建構 | O(n × \|Σ\|) | n = corpus 長度 |
| SAM 後綴識別 | O(\|word\| × n_patterns) | 含驗證 |
| 互動模式查詢 | O(\|key\|) 或 O(n × \|key\|²) | 精確或模糊 |

---

## 設計思路

### 為什麼是 C？

- 完全控制記憶體配置（pool 設計）
- 直接操作 byte 流（UTF-8 + BOM 處理）
- 沒有 hidden allocations，效能可預期

### 取捨

- **Pool 不支援 free**：以重新配置 + 浪費部份 pool 空間換取簡潔，且不犯 use-after-free
- **Radix Tree 不支援 delete**：日文詞典極少需要刪除詞條
- **SAM 用固定大小 ASCII 表**：日文後綴以羅馬字處理，字母表簡化至 27（a-z + `$`）
- **互動模式每次重建 SAM**：適合示範用途；正式應用應將 SAM 預先建構為全域物件
- **godan_ru 與 ichidan 需手動標記**：是日語本身的語言學限制，無法從羅馬字自動判斷
