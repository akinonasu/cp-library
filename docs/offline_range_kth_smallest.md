---
title: 区間の k 番目の値（クエリ一括処理）
documentation_of: //data_structure/offline_range_kth_smallest.hpp
library_category: data_structure/range_query
redirect_from:
- /data_structure/range_query/offline_range_kth_smallest.hpp.html
- /lightweight/data_structure/range_query/offline_range_kth_smallest.hpp.html
---

```cpp
#include "lib/data_structure/offline_range_kth_smallest.hpp"

std::vector<int> a{3, 1, 4, 1, 5};
std::vector<std::array<int, 3>> queries{
	{0, 5, 2},
	{1, 4, 1},
	{2, 5, 0}
};
auto answer = offline_range_kth_smallest(a, queries); // {3, 1, 1}
```

| 操作 | 内容・返り値 |
| --- | --- |
| `offline_range_kth_smallest(values,queries)` | 各 `{l,r,k}` について、`values[l,r)` の小さい方から 0 始まりで `k` 番目の値を、問い合わせと同じ順の `vector<T>` で返す |

`values` は `vector<T>`、`queries` は `span<const array<int,3>>` です。`vector<array<int,3>>` をそのまま渡せます。元の列と問い合わせは変更しません。

列長を $$N$$ として、各問い合わせは $$0\le l<r\le N$$、$$0\le k<r-l$$ を満たすものとします。列長・問い合わせ数は `INT_MAX` 以下です。空の問い合わせ列には空の `vector` を返します。

`T` はコピーと大小・等値比較が可能な型とします。整数・文字列などに対応します。問い合わせを一件ずつ処理する場合は [Wavelet Matrix](wavelet_matrix.md) を使えます。

列長を $$N$$、問い合わせ数を $$Q$$、異なる値の数を $$\sigma$$ とし、比較・コピーを定数時間とします。64 bit 以下の整数列では $$O((N+Q)\log\sigma)$$ 時間、それ以外の型では $$O(N\log N+Q\log\sigma)$$ 時間、追加空間は $$O(N+Q)$$ です。値が一種類以下の場合は $$O(N+Q)$$ 時間です。
