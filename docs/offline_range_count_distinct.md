---
title: 区間の異なる値の個数（クエリ一括処理）
documentation_of: //data_structure/offline_range_count_distinct.hpp
library_category: data_structure/range_query
redirect_from:
- /data_structure/range_query/offline_range_count_distinct.hpp.html
- /lightweight/data_structure/range_query/offline_range_count_distinct.hpp.html
---

```cpp
#include "lib/data_structure/offline_range_count_distinct.hpp"

OfflineRangeCountDistinct<int> query(std::vector<int>{2, 1, 2, 3});
query.add_query(0, 3);
query.add_query(1, 4);
auto answer = query.solve(); // {2, 3}
```

| 操作 | 内容・返り値 |
| --- | --- |
| `OfflineRangeCountDistinct<T,Hash,Equal>(values)` / `build(values)` | 初期列を設定し、登録済みの問い合わせを消去 |
| `size()` | 列の長さ |
| `reserve(q)` | 問い合わせ数を予約 |
| `add_query(l,r)` | 半開区間 `[l,r)` の問い合わせを追加し、答えの添字を返す |
| `solve()` | 異なる値の個数を登録順の `vector<int>` で返す |

添字は 0 始まり、`0 <= l <= r <= size()` とします。空区間の答えは `0` です。`solve()` は登録内容を変更せず、繰り返し呼び出せます。入力配列は変更せず、構築後に破棄できます。`T` はコピー可能とし、`Hash` と `Equal` は `std::unordered_map` と同じ条件を満たすものを渡します。列長・問い合わせ数は `INT_MAX` 以下とします。

列長を $$N$$、問い合わせ数を $$Q$$ とします。通常のハッシュ計算・比較を定数時間として、構築は $$O(N)$$、問い合わせの登録は償却 $$O(1)$$、`solve()` は期待 $$O((N+Q)\log N)$$ 時間、空間は $$O(N+Q)$$ です。
