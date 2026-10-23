---
title: 区間頻度（一点更新・クエリ一括処理）
documentation_of: //data_structure/range_query/offline_range_frequency.hpp
---

```cpp
#include "lib/data_structure/range_query/offline_range_frequency.hpp"

OfflineRangeFrequency<int> query(std::vector<int>{2, 1, 2, 3});
int a = query.add_query(0, 4, 2);
query.set(1, 2);
int b = query.add_query(0, 3, 2);
auto answer = query.solve(); // answer[a] = 2, answer[b] = 3
```

半開区間 `[l,r)` にある `value` の個数を求めます。更新と問い合わせを呼び出した順に登録し、`solve()` で全問い合わせの答えを登録順に返します。登録後に `solve()` を繰り返し呼び出せます。

| 操作 | 内容・返り値 |
| --- | --- |
| `OfflineRangeFrequency<T,Hash,Equal>(values)` / `build(values)` | 初期列を設定し、登録済みの操作を消去 |
| `size()` | 列の長さ |
| `reserve(operations)` | 登録する更新・問い合わせの予定数を予約 |
| `set(p,value)` | 位置 `p` を `value` に代入 |
| `add_query(l,r,value)` | 問い合わせを追加し、答えの添字を返す |
| `solve()` | `vector<int>` で答えを返す |

添字は 0 始まりで、更新は `0 <= p < size()`、問い合わせは `0 <= l <= r <= size()` とします。空区間や指定した値がない区間の答えは 0 です。入力配列は変更せず、登録した操作の後に破棄できます。`T` はコピー可能とし、`Hash` と `Equal` は `std::unordered_map` と同じ条件を満たすものを渡します。既定では `std::hash<T>`・`std::equal_to<T>` を使います。列長・問い合わせ数・登場する値の種類数は `INT_MAX` 以下とします。

列長を $$N$$、更新数を $$U$$、問い合わせ数を $$Q$$ とします。通常のハッシュ計算・比較を定数時間として、構築は期待 $$O(N)$$、登録は償却期待 $$O(1)$$、`solve()` は $$O((N+U+Q)\log N)$$ 時間、空間は $$O(N+U+Q)$$ です。更新がない場合、`solve()` は $$O(N+Q)$$ 時間です。
