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
| `reserve(q)` | 非負の問い合わせ数 `q` を予約 |
| `add_query(l,r,upper)` | `[l,r)` にある `value < upper` の要素の問い合わせを追加し、答えの添字を返す |
| `solve()` | `{和,個数}` を登録順の `vector<pair<Sum,int>>` で返す |

添字は 0 始まり、`0 <= l <= r <= size()` とします。値が `upper` と等しい要素は含めません。空区間や条件に合う要素がない場合の答えは `{0,0}` です。`solve()` は登録内容を変更せず、繰り返し呼び出せます。入力配列は変更せず、構築後に破棄できます。

`T` はコピー・大小比較が可能で、`Sum` に変換できる型とします。`Sum` は `0` による初期化と加減算に対応し、集計結果が収まる型を指定します。既定の `Sum` は、64 bit 未満の整数 `T` では `int64_t`、64 bit 整数では `__int128_t`、それ以外では `T` です。列長・問い合わせ数は `INT_MAX` 以下とします。

列長を $$N$$、問い合わせ数を $$Q$$ とし、値のコピー・破棄・演算を定数時間として、構築は $$O(N)$$、`size()` は $$O(1)$$、登録は償却 $$O(1)$$ です。`reserve(q)` は容量が十分なら $$O(1)$$、再確保する場合は $$O(Q)$$ 時間です。`build` による再構築には、旧配列と旧問い合わせの値を破棄する時間も加わります。

`solve()` は $$O((N+Q)\log(N+Q))$$ 時間・$$O(N+Q)$$ 追加空間です。問い合わせがなければ $$O(1)$$、空列なら $$O(Q)$$ 時間です。空の構築や要素がない領域の予約にも定数時間かかります。保持空間は内部配列の確保容量の合計を $$C$$ として $$O(C)$$ です。`reserve` と再構築は容量を保持することがあります。

「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
