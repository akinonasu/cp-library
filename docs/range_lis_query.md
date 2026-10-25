---
title: Range LIS Query（区間の最長増加部分列）
documentation_of: //data_structure/range_lis_query.hpp
library_category: data_structure/range_query
redirect_from:
- /data_structure/range_query/range_lis_query.hpp.html
- /lightweight/data_structure/range_query/range_lis_query.hpp.html
---

変更のない列に対して、半開区間 `[l,r)` の最長増加部分列の長さを求めます。問い合わせは構築後に順次指定でき、順列以外の列や重複する値にも対応します。

```cpp
#include "lib/data_structure/range_lis_query.hpp"

std::vector<int> a{3, 1, 2, 2, 4};
RangeLISQuery<> tree(a);
int x = tree.query(0, 5); // 3: 1, 2, 4
int y = tree.query(1, 4); // 2: 1, 2
int z = tree.query(2, 2); // 0

RangeLISQuery<false> weak(a);
int w = weak.query(0, 5); // 4: 1, 2, 2, 4

RangeLISQuery<> decreasing(a, std::greater<>{});
int d = decreasing.query(0, 5); // 2: 3, 1
```

既定は狭義増加です。`RangeLISQuery<false>` は等しい値を続けて選べる広義増加を求めます。比較関数には厳密弱順序を指定します。狭義では `compare(x,y)`、広義では `!compare(y,x)` を満たす順に選びます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RangeLISQuery<Strict=true>()` | 空の列を作る | $$O(1)$$ |
| `RangeLISQuery<Strict>(a,compare=std::less<>{})` | 列 `a` から構築する | $$O(n\log^2 n)$$ |
| `build(a,compare=std::less<>{})` | 列と比較を指定して構築し直す | $$O(n\log^2 n)$$ |
| `query(l,r) const` | `[l,r)` の最長増加部分列の長さを `int` で返す | $$O(\log n)$$ |
| `size()` / `empty()` | 列の長さ / 空列かどうか | $$O(1)$$ |
| `clear()` | 空列に戻す | $$O(n)$$ |
| `swap(other)` | 同じ `Strict` の構造と内容を交換する | $$O(1)$$ |

`a` には const でサイズ取得とランダムアクセスができる範囲を渡します。`vector`・`array`・C 配列・`span`・`deque`・`string_view` などを利用できます。要素のコピーや値の加減算は不要で、比較できれば使えます。元の列を変更・破棄しても、構築済みの答えには影響しません。

区間は `0 <= l <= r <= size()`、列の長さは `INT_MAX` 以下とします。空の列や空区間の答えは 0 です。部分列そのものの復元や、構築後の要素更新には対応しません。列全体の長さや復元だけが必要な場合は [LIS](lis.md) を利用できます。

参考: [Library Checker: Static Range LIS Query](https://judge.yosupo.jp/problem/static_range_lis_query)、[Tiskin: Semi-local string comparison](https://arxiv.org/abs/0707.3619)。

列の長さを $$n$$ とし、比較を定数時間とすると、前計算は $$O(n\log^2 n)$$ 時間、問い合わせは $$O(\log n)$$ 時間・$$O(1)$$ 追加空間です。保持空間は $$O(n\log n)$$ bit、前計算の作業領域は $$O(n)$$ word です。
