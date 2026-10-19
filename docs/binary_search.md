---
title: "単調な述語の境界探索"
documentation_of: //components/binary_search.hpp
---

```cpp
#include "lib/components/binary_search.hpp"
#include <vector>

std::vector<int> a{1, 3, 3, 7};
int lower = binary_search_detail::partition_point(4, [&](int i) {
	return a[i] < 3;
}); // 1
int upper = binary_search_detail::partition_point(4, [&](int i) {
	return a[i] <= 3;
}); // 3
```

添字に対する述語が、ある境界を境に `true` から `false` へ変わるときに使う内部部品です。LIS の二分探索でも使用します。比較結果を次の探索位置の加算に変換します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `binary_search_detail::partition_point(n,predicate)` | `0 <= i < n` で最初に `predicate(i)` が false になる添字。全て true なら `n` | $$O(\log n)$$ |

`n >= 0` とし、述語は探索中に結果が変わらないものを使います。引数として渡す添字は必ず `[0,n)` 内です。`n == 0` の場合は述語を呼ばずに `0` を返します。

述語を $$O(1)$$ として、時間は $$O(\log n)$$、追加空間は $$O(1)$$ です。空の範囲は $$O(1)$$ 時間です。
