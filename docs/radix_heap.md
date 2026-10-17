---
title: "Radix Heap"
documentation_of: //components/RadixHeap.hpp
redirect_from: /data_structure/RadixHeap.hpp.html
---

```cpp
#include "lib/components/RadixHeap.hpp"

RadixHeap<std::uint64_t, int> heap;
heap.push({10, 3});
auto [key, value] = heap.pop_value();
```

| 操作 | 内容・返り値 |
| --- | --- |
| `RadixHeap(minimum = 0)` | 初期下限を指定して空ヒープを構築 |
| `push({key,value})` | `key >= last()` を満たす要素を追加 |
| `top()` | 最小要素の const 参照。非空であること。下限も更新 |
| `pop()` / `pop_value()` | 最小要素を削除／取得して削除。非空であること |
| `last()` | 現在のキーの下限 |
| `size()` / `empty()` | 要素数／空判定 |
| `clear(minimum = 0)` | 空に戻し、キーの下限を設定 |

`Key` は 64 bit 以下の整数型です。`push({key, value})` のキーは `last()` 以上にします。初期下限は `RadixHeap(minimum)` で指定でき、既定値は 0 です。`top()` も下限を更新します。

`top()` は最小要素の参照、`pop()` は最小要素の削除、`pop_value()` は取得と削除です。`size()`・`empty()`・`clear(minimum = 0)` も使えます。

キーのビット数を $$w$$、要素数を $$n$$ として、`push` は償却 $$O(1)$$、取り出しは償却 $$O(w)$$、空間は $$O(n+w)$$ です。
