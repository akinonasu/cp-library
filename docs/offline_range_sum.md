---
title: 値の上限付き区間和（クエリ一括処理）
documentation_of: //data_structure/offline_range_sum.hpp
library_category: data_structure/range_query
redirect_from:
- /data_structure/range_query/offline_range_sum.hpp.html
- /lightweight/data_structure/range_query/offline_range_sum.hpp.html
---

```cpp
#include "lib/data_structure/offline_range_sum.hpp"

OfflineRangeSum<int> query(std::vector<int>{4, 1, 4, 2});
query.add_query(0, 4, 4);
query.add_query(1, 3, 5);
auto answer = query.solve(); // { {3, 2}, {5, 2} } : {和, 個数}
```

| 操作 | 内容・返り値 |
| --- | --- |
| `OfflineRangeSum<T,Sum>(values)` / `build(values)` | 初期列を設定し、登録済みの問い合わせを消去 |
| `size()` | 列の長さ |
| `reserve(q)` | 問い合わせ数を予約 |
| `add_query(l,r,upper)` | `[l,r)` にある `value < upper` の要素の問い合わせを追加し、答えの添字を返す |
| `solve()` | `{和,個数}` を登録順の `vector<pair<Sum,int>>` で返す |

添字は 0 始まり、`0 <= l <= r <= size()` とします。値が `upper` と等しい要素は含めません。空区間や条件に合う要素がない場合の答えは `{0,0}` です。`solve()` は登録内容を変更せず、繰り返し呼び出せます。入力配列は変更せず、構築後に破棄できます。

`T` はコピー・大小比較が可能で、`Sum` に変換できる型とします。`Sum` は `0` による初期化と加減算に対応し、集計結果が収まる型を指定します。既定の `Sum` は、64 bit 未満の整数 `T` では `int64_t`、64 bit 整数では `__int128_t`、それ以外では `T` です。列長・問い合わせ数は `INT_MAX` 以下とします。

列長を $$N$$、問い合わせ数を $$Q$$ とし、要素の演算を定数時間として、構築は $$O(N)$$、登録は償却 $$O(1)$$、`solve()` は $$O((N+Q)\log(N+Q))$$ 時間、空間は $$O(N+Q)$$ です。
