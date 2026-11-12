---
title: 静的区間の転倒数
documentation_of: //data_structure/static_range_inversions.hpp
redirect_from:
- /sequence/static_range_inversions.hpp.html
- /lightweight/sequence/static_range_inversions.hpp.html
library_category: data_structure/range_query
---

数列と半開区間の問い合わせをまとめて渡し、各区間の転倒数を返します。同じ値の組は数えず、入力は変更しません。

```cpp
#include "lib/data_structure/static_range_inversions.hpp"

std::vector<int> a{3, 1, 2, 1};
std::vector<std::pair<int,int>> ranges{ {0,4}, {1,4}, {2,2} };
auto counts = static_range_inversion_counts(a, ranges); // {4,1,0}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `static_range_inversion_counts(a,ranges,compare = std::less<>{})` | 各 `[l,r)` 内の `i < j` かつ `compare(a[j],a[i])` となる組数を、入力区間順の `vector<int64_t>` で返す | $$O(n\sqrt n+n\sqrt q+q)$$ |

各区間は `0 <= l <= r <= a.size()` とします。空区間の答えは 0 です。列長と区間数は `INT_MAX` 以下とし、この範囲では答えは符号付き 64 bit に収まります。

入力の数列には、読み取り専用の状態でも要素数を取得でき、先頭からの位置を指定して任意の要素を読み取れる列を使えます。`vector`・`array`・`span`・`deque`・C 配列や、コピー不能な要素にも対応します。比較関数は、左の引数が右の引数より前に来るかを返します。比較では、自分自身には `false` を返し、`a` が `b` より前、`b` が `c` より前なら `a` が `c` より前となるようにします。どちら向きの比較も `false` のときは同値とみなし、`a` と `b`、`b` と `c` が同値なら `a` と `c` も同値になるようにします。比較上同値な値同士は数えず、`std::greater<>{}` なら左の値が右の値より小さい組を数えます。

列長を $$n$$、区間数を $$q$$ として、時間は $$O(n\sqrt n+n\sqrt q+q)$$、追加空間は $$O(n+q)$$ です。問い合わせが 1 個の場合は、その区間に対する [転倒数](https://qandori.github.io/cp-library/sequence/inversion_count.hpp.html) の計算を使います。比較と任意位置の要素取得を定数時間とします。
