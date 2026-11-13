---
title: 区間頻度（一点更新・クエリ一括処理）
documentation_of: //data_structure/offline_range_frequency.hpp
library_category: data_structure/range_query
redirect_from:
- /data_structure/range_query/offline_range_frequency.hpp.html
- /lightweight/data_structure/range_query/offline_range_frequency.hpp.html
---

```cpp
#include "lib/data_structure/offline_range_frequency.hpp"

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
| `reserve(operations)` | 非負の更新・問い合わせの予定数 `operations` を予約 |
| `set(p,value)` | 位置 `p` を `value` に代入 |
| `add_query(l,r,value)` | 問い合わせを追加し、答えの添字を返す |
| `solve()` | `vector<int>` で答えを返す |

添字は 0 始まりで、更新は `0 <= p < size()`、問い合わせは `0 <= l <= r <= size()` とします。空区間や指定した値がない区間の答えは 0 です。入力配列は変更せず、登録した操作の後に破棄できます。`T` はコピー可能とし、`Hash` と `Equal` は `std::unordered_map` と同じ条件を満たすものを渡します。`Hash(x)` はハッシュ値、`Equal(x,y)` は 2 つの値を同じとみなすかを返す関数で、同じとみなす値のハッシュ値は等しくしてください。既定では `std::hash<T>`・`std::equal_to<T>` を使います。列長・問い合わせ数・登場する値の種類数は `INT_MAX` 以下とします。

列長を $$N$$、更新数を $$U$$、問い合わせ数を $$Q$$、内部配列とハッシュ表の確保容量の合計を $$C$$ とします。値のコピー・破棄・ハッシュ計算・比較を定数時間として、構築は期待 $$O(N)$$、`size()` は $$O(1)$$、登録は償却期待 $$O(1)$$ です。`reserve(operations)` は容量が十分なら $$O(1)$$、再確保する場合は $$O(U+Q)$$ 時間です。`build` による再構築は、旧領域の消去を含めて期待 $$O(C+N)$$ 時間です。

`solve()` は $$O((N+U+Q)\log N)$$ 時間・$$O(N+U+Q)$$ 追加空間です。更新がない場合は $$O(N+Q)$$、$$N\le1$$ では $$O(U+Q)$$、問い合わせがなければ $$O(1)$$ 時間です。空の構築や要素がない領域の予約にも定数時間かかります。保持空間は $$O(C)$$ で、`reserve` と再構築は容量を保持することがあります。

「期待」はハッシュ値が偏らないときの平均時間を、「償却」は一連の操作の合計時間から求めた 1 回あたりの計算量を表します。
