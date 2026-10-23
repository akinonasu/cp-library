---
title: 区間アフィン変換・区間和
documentation_of: //data_structure/segment_tree/range_affine_range_sum.hpp
---

区間の各値を `a*x+b` に変更し、区間和を求めます。

```cpp
#include "lib/data_structure/segment_tree/range_affine_range_sum.hpp"

RangeAffineRangeSum<> values(std::vector<mint>{1, 2, 3});
values.apply(0, 2, mint(2), mint(1)); // {3,5,3}
auto sum = values.prod(0, 3);       // 11
values.set(1, mint(7));
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RangeAffineRangeSum<Mint=mint>()` | 空の構造を作成 | $$O(1)$$ |
| `RangeAffineRangeSum(values)` | `vector<Mint>` から構築 | $$O(n)$$ |
| `RangeAffineRangeSum(n,value=Mint(0))` | 長さ `n`、全要素 `value` で構築 | $$O(n)$$ |
| `size()` | 要素数 | $$O(1)$$ |
| `apply(l,r,a,b)` / `apply(p,a,b)` | `[l,r)`／位置 `p` を `a*x+b` に変換 | $$O(\log n)$$ |
| `prod(l,r)` / `sum(l,r)` | `[l,r)` の和。空区間なら 0 | $$O(\log n)$$ |
| `all_prod()` / `all_sum()` | 全要素の和。空なら 0 | $$O(1)$$ |
| `get(p)` / `operator[](p)` | 位置 `p` の値 | $$O(\log n)$$ |
| `set(p,value)` | 位置 `p` へ代入 | $$O(\log n)$$ |
| `swap(other)` | 内容を交換 | $$O(1)$$ |

区間は `0 <= l <= r <= size()`、位置は `0 <= p < size()` とします。長さは `INT_MAX` 以下です。コピーは独立した構造を作り、移動元は再利用できる空の構造になります。取得操作は `const` オブジェクトから呼べます。

`Mint` には通常の modint を指定します。静的な奇数法が $$2^{30}$$ 未満で AVX2 が使える場合は専用処理、それ以外は遅延セグメント木を使います。既定の `mint` と `atcoder::modint1000000007` に対応します。法を実行中に変更しないでください。

構築・使用空間は $$O(n)$$、更新と区間の取得は $$O(\log n)$$ 時間です。modint の演算を定数時間とします。
