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

| 操作 | 内容・返り値 |
| --- | --- |
| `PredecessorSet(n = 0,filled = false)` | 範囲 `[0,n)` の空集合。`filled=true` なら全要素を入れる |
| `assign(n,filled = false)` | 範囲と内容を再設定 |
| `universe_size()` | 管理する範囲の長さ |
| `size()` / `empty()` | 要素数／空判定 |
| `contains(p)` | `p` が含まれるか。`0 <= p < n` が必要 |
| `insert(p)` | 要素を追加。既に含まれる場合は false |
| `erase(p)` | 要素を削除。含まれない場合は false |
| `next(p)` | `p` 以上の最小要素。なければ `n` |
| `prev(p)` | `p` 以下の最大要素。なければ -1 |
| `clear()` | 範囲を保って空集合にする |
| `memory_usage()` | 確保済みの動的領域のバイト数 |

`n` は非負の int、挿入・削除の対象は `[0,n)` としてください。`next`・`prev` の引数は範囲外でも使えます。コピー・ムーブに対応し、移動元は範囲長 0 の空集合になります。

構築・`assign`・`clear` は $$O(1+n/64)$$ 時間、保持空間は $$O(1+n/64)$$ 語です。`insert`・`erase`・`next`・`prev`・`memory_usage` は $$O(\log_{64}n)$$、他の問い合わせは $$O(1)$$ です。$$n\leq1$$ では各操作を $$O(1)$$ とします。再設定時には旧範囲の領域の破棄も必要です。
