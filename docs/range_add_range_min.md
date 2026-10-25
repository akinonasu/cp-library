---
title: 区間加算・区間最小値
documentation_of: //data_structure/range_add_range_min.hpp
library_category: data_structure/segment_tree
redirect_from:
- /data_structure/segment_tree/range_add_range_min.hpp.html
- /lightweight/data_structure/segment_tree/range_add_range_min.hpp.html
---

区間の全要素への加算と区間最小値の取得に対応します。

```cpp
#include "lib/data_structure/range_add_range_min.hpp"

RangeAddRangeMin<> values(std::vector<std::int64_t>{3, 1, 5});
values.add(0, 2, 4);        // {7,5,5}
auto a = values.prod(0, 3); // 5
values.set(2, -1);
auto b = values.all_min(); // -1
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RangeAddRangeMin<T=std::int64_t>()` | 空の構造を作成 | $$O(1)$$ |
| `RangeAddRangeMin(values)` | `vector<T>` から構築 | $$O(n)$$ |
| `RangeAddRangeMin(n,value=0)` | 長さ `n`、全要素 `value` で構築 | $$O(n)$$ |
| `build(values)` | `vector<T>` から作り直す | $$O(n)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `add(l,r,value)` / `apply(l,r,value)` | `[l,r)` の各値に `value` を加算 | $$O(\log n)$$ |
| `prod(l,r)` / `min(l,r)` | `[l,r)` の最小値 | $$O(\log n)$$ |
| `all_prod()` / `all_min()` | 全要素の最小値 | $$O(1)$$ |
| `get(p)` | 位置 `p` の値 | $$O(\log n)$$ |
| `set(p,value)` | 位置 `p` に代入 | $$O(\log n)$$ |
| `values()` | 現在の値を `vector<T>` で返す | $$O(n)$$ |
| `memory_usage()` | 確保した配列領域のバイト数 | $$O(1)$$ |
| `swap(other)` | 内容を交換 | $$O(1)$$ |

`T` は 64 bit 以下の標準の符号付き整数型です。各更新後のすべての要素が `T` の範囲に収まるように使ってください。区間は `0 <= l <= r <= size()`、位置は `0 <= p < size()`、長さは `INT_MAX` 以下です。空区間・空の構造の最小値は `std::numeric_limits<T>::max()` を返します。この値を通常の要素として格納することもできます。

取得操作は `const` オブジェクトから呼べます。コピーは独立した構造を作り、移動元は再利用できる空の構造になります。

構築・使用空間・`values` は $$O(n)$$、更新・区間最小値・一点の取得や代入は $$O(\log n)$$ 時間です。64 bit 整数では AVX-512 を有効にしたビルドで SIMD 処理を使い、それ以外の環境でも同じ結果を返します。
