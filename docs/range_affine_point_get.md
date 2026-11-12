---
title: 区間アフィン変換・一点取得
documentation_of: //data_structure/range_affine_point_get.hpp
library_category: data_structure/segment_tree
---

区間の各値を `a*x+b` に変更し、一点の値を取得します。区間和が必要な場合は [RangeAffineRangeSum](range_affine_range_sum.md) を使います。

```cpp
#include "lib/data_structure/range_affine_point_get.hpp"
#include <cassert>
#include <vector>

int main() {
	RangeAffinePointGet<> values(std::vector<mint>{1, 2, 3});
	values.apply(0, 2, mint(2), mint(1));
	assert(values.get(0) == mint(3) && values[1] == mint(5));
	values.apply(1, 3, mint(0), mint(7));
	values.set(2, mint(9));
	assert(values[1] == mint(7) && values[2] == mint(9));
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RangeAffinePointGet<Mint=mint>()` | 空の構造を作成 | $$O(1)$$ |
| `RangeAffinePointGet(values)` | `const vector<Mint>&` から独立した構造を作成 | $$O(n)$$ |
| `RangeAffinePointGet(n,value=Mint(0))` | 長さ `n`、全要素 `value` で構築 | $$O(n)$$ |
| `size()` / `empty()` | `int` の要素数／空かを表す `bool` | $$O(1)$$ |
| `apply(l,r,a,b)` / `apply(p,a,b)` | `[l,r)`／位置 `p` を `a*x+b` に変換。返り値は `void` | $$O(\log n)$$ |
| `get(p)` / `operator[](p)` | 位置 `p` の値を `Mint` で返す | $$O(\log n)$$ |
| `set(p,value)` | 位置 `p` へ代入。返り値は `void` | $$O(\log n)$$ |
| `swap(other)` | 内容を交換。返り値は `void` | $$O(1)$$ |

添字は 0 始まりで、区間は `0 <= l <= r <= size()`、位置は `0 <= p < size()` とします。空区間への更新は何もしません。長さは `0 <= n <= INT_MAX/4` です。`a=0` を含む変換を呼び出した順に適用します。取得操作は `const` オブジェクトから呼べます。

`Mint` には整数の `0`・`1` からの構築、コピー、加算、乗算が必要です。加算と乗算が環の法則を満たす型を使います。既定の `mint` のほか、通常の modint を指定できます。法を実行中に変更しないでください。

`mod()` と `val()` を提供する型では、`mod()` を法、`val()` を0以上かつ法未満の代表元とします。

コピーは独立した構造を作り、移動元は再利用できる空の構造になります。コピーの時間・使用空間と構築後の使用空間は $$O(n)$$ です。上記の計算量では `Mint` の演算を定数時間とします。
