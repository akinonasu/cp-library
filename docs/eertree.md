---
title: "Eertree（回文木）"
documentation_of: //string/eertree.hpp
---

```cpp
#include "lib/string/eertree.hpp"

Eertree tree("ababa");
auto count = tree.occurrences();
int v = tree.longest_palindrome();
auto [l, r] = tree.first_occurrence(v);
tree.push_back('b');
```

| 関数・定数 | 内容・返り値 |
| --- | --- |
| `Eertree()` / `Eertree(s)` | 空の木／`string_view` の文字列から構築 |
| `clear()` | 空の状態に戻す。確保済みの領域は保持 |
| `reserve(n)` | 長さ `n` の文字列に必要な領域を確保 |
| `push_back(c)` | 末尾に 1 バイトを追加し、その時点の最長回文接尾辞の頂点を返す |
| `append(s)` | `string_view` を末尾に追加 |
| `size()` / `empty()` | 追加済みの文字数／空かどうか |
| `node_count()` | 特別な根 2 個を含む頂点数 |
| `distinct_count()` | 相異なる空でない回文部分文字列の数 |
| `last()` | 現在の最長回文接尾辞の頂点。空文字列では `even_root` |
| `longest_palindrome()` | 最長回文部分文字列の頂点。同じ長さでは先に現れたもの。空文字列では `even_root` |
| `palindrome_count()` | 空でない回文部分文字列の総出現回数。重なる出現も数える。`uint64_t` |
| `length(v)` | 頂点 `v` の回文の長さ |
| `suffix_link(v)` | 最長の真の回文接尾辞の頂点。存在しなければ `even_root` |
| `parent(v)` | 先頭と末尾の文字を除いた回文の頂点。長さ 1 では `odd_root`、長さ 2 では `even_root` |
| `suffix_count(v)` | 頂点 `v` の空でない回文接尾辞の数。自身も含む。根では 0 |
| `terminal_count(v)` | 各接頭辞の最長回文接尾辞が `v` となった回数 |
| `first_occurrence(v)` | 最初に現れる半開区間 `[l,r)` を `pair<int,int>` で返す |
| `transition(v,c)` | 回文の両端に `c` を付けた回文の頂点。未登場なら -1。`odd_root` からは長さ 1、`even_root` からは長さ 2 への遷移 |
| `prefix_nodes()` | 各非空接頭辞の最長回文接尾辞を、`const vector<int>&` で返す。要素 `i` は長さ `i+1` の接頭辞 |
| `occurrences()` | 各頂点の総出現回数を `vector<uint64_t>` で返す。根の要素は 0 |
| `odd_root` / `even_root` | 長さ -1 の根 0／長さ 0 の根 1 |

空でない回文の頂点番号は 2 から初出順です。1 文字の追加で増える頂点は高々 1 個で、追加済みの頂点番号は変わりません。suffix link と親は、対応する頂点より小さい番号を指します。`parent` と `first_occurrence` には空でない回文の頂点を渡してください。根の suffix link は両方とも `odd_root` を指します。

文字列はバイト単位で扱い、NUL・128 以上の値を含め全 256 種類のバイトに対応します。文字数は `INT_MAX-2` 以下としてください。削除・先頭への追加には対応しません。

`suffix_count(last())` は現在の末尾で終わる回文の数です。出現回数の取得は木を変更しないため、取得後も追加でき、繰り返し取得しても二重に加算されません。`prefix_nodes()` の参照先への要素追加・参照の有効期間には、通常の `vector` と同じ規則が適用されます。

根の遷移は配列、それ以外は頂点と一体化した連結リストです。番兵付きの 16 bit 配列で文字の比較を行い、suffix link の探索中の範囲確認を省きます。構築は決定的です。

文字数を $$n$$、登場するバイトの種類数を $$\sigma\leq256$$、回文の種類数を $$d\leq n$$ とします。構築・末尾追加の合計は $$O(n\sigma)$$ 時間、空間は $$O(n)$$ です。`push_back` は償却 $$O(\sigma)$$ 時間、`transition` は $$O(\sigma)$$ 時間、`occurrences` は $$O(d)$$ 時間・空間です。`reserve`・`clear` は既存領域の処理を含めて $$O(n)$$ 時間、その他の取得は $$O(1)$$ 時間です。
