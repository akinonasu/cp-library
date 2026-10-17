---
title: "Predecessor Set"
documentation_of: //components/predecessor_set.hpp
---

```cpp
#include "lib/components/predecessor_set.hpp"

PredecessorSet set(100);
set.insert(3);
set.insert(70);
auto a = set.next(4); // 70
auto b = set.prev(69); // 3
```

固定した整数範囲 `[0,n)` の集合を階層ビット列で管理します。各階層は、1 つ下の階層の 64 bit の語が空でないかを表します。

全階層を連続した配列で保持する 64 分木です。[Fast Set](../data_structure/fast_set.hpp.html) と実装・API を共通化しています。

| 操作 | 内容・返り値 |
| --- | --- |
| `PredecessorSet(n = 0,filled = false)` | 範囲 `[0,n)` の空集合。`filled=true` なら全要素を入れる |
| `PredecessorSet(bits)` | `string_view` の 0/1 列から構築 |
| `PredecessorSet(n,words)` | `span<const uint64_t>` のビット列から構築 |
| `assign(n,filled = false)` | 範囲と内容を再設定 |
| `assign(bits)` / `assign(n,words)` | 0/1 列／整数のビット列から再構築 |
| `universe_size()` | 管理する範囲の長さ |
| `size()` / `empty()` | 要素数／空判定 |
| `contains(p)` | `p` が含まれるか。`0 <= p < n` が必要 |
| `insert(p)` | 要素を追加。既に含まれる場合は false |
| `erase(p)` | 要素を削除。含まれない場合は false |
| `set(p,present)` | true なら追加、false なら削除。集合が変化した場合だけ true |
| `next(p)` | `p` 以上の最小要素。なければ `n` |
| `prev(p)` | `p` 以下の最大要素。なければ -1 |
| `min()` / `max()` | 最小／最大要素。空ならそれぞれ `n`／-1 |
| `for_each(l,r,f)` / `for_each(f)` | 区間／全体の要素を昇順に `f(int)` へ渡す |
| `to_vector()` | 全要素を昇順の `vector<int>` で返す |
| `words()` | 最下層の読み取り専用 `span<const uint64_t>` |
| `clear()` / `fill()` | 範囲と容量を保って空集合／全体集合にする |
| `swap(other)` | 範囲と内容を交換 |
| `memory_usage()` | 確保済みの動的領域のバイト数 |

`n` は `0..INT_MAX`、挿入・削除の対象は `[0,n)` としてください。`next`・`prev` の引数は範囲外でも使えます。コピー・ムーブに対応し、移動元は範囲長 0 の空集合になります。

0/1 列は `'0'`・`'1'` だけを含む列です。整数のビット列の長さは `ceil(n/64)` で、位置 `i` は `words[i/64]` の下から `i%64` bit です。最後の語の範囲外の bit は無視します。入力はコピーし、自分自身の `words()` からも再構築できます。返した span は更新で内容が変化し、再構築・移動・破棄の後は使えません。一時オブジェクトへの `words()` は禁止しています。列挙は半開区間 `[l,r)` で、コールバック中に同じ集合を変更しないでください。

0/1 列の変換には x86_64 の AVX2 を実行時に判定して使います。使えない環境では通常の整数演算で動作します。

整数・整数のビット列からの構築・`assign` と `clear`・`fill` は $$O(1+n/64)$$、0/1 列からは $$O(n)$$ 時間です。`insert`・`erase`・`set`・`next`・`prev`・`min`・`max` は $$O(\log_{64}n)$$、列挙する要素数を $$k$$ として列挙・`to_vector` は $$O((k+1)\log_{64}n)$$、他の問い合わせ・`swap` は $$O(1)$$ です。$$n\leq1$$ の検索・更新は $$O(1)$$ です。保持空間は $$O(1+n/64)$$ 語、容量を含む確保空間は過去の最大範囲長 $$M$$ に対して $$O(1+M/64)$$ 語です。
