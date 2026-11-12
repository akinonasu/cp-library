---
title: Range LIS Query（区間の最長増加部分列）
documentation_of: //data_structure/range_lis_query.hpp
library_category: data_structure/range_query
redirect_from:
- /data_structure/range_query/range_lis_query.hpp.html
- /lightweight/data_structure/range_query/range_lis_query.hpp.html
---

変更のない列に対して、半開区間 `[l,r)` の最長増加部分列の長さを求めます。問い合わせは構築後に順次指定でき、順列以外の列や重複する値にも対応します。全区間が決まっている場合は、一括処理の `range_lis_queries` と `range_lis_queries_sqrt` も使えます。

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

既定は、選んだ値が順に大きくなる狭義増加です。`RangeLISQuery<false>` は等しい値を続けて選べる広義増加を求めます。比較関数 `compare(x,y)` は `x` が `y` より前に来るかを返します。比較では、自分自身には `false` を返し、`a` が `b` より前、`b` が `c` より前なら `a` が `c` より前となるようにします。どちら向きの比較も `false` のときは同値とみなし、`a` と `b`、`b` と `c` が同値なら `a` と `c` も同値になるようにします。狭義では `compare(x,y)`、広義では `!compare(y,x)` を満たす順に選びます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RangeLISQuery<Strict=true>()` | 空の列を作る | $$O(1)$$ |
| `RangeLISQuery<Strict>(a,compare=std::less<>{})` | 列 `a` から構築する | $$O(n\log^2 n)$$ |
| `build(a,compare=std::less<>{})` | 列と比較を指定して構築し直す | $$O(n\log^2 n)$$ |
| `query(l,r) const` | `[l,r)` の最長増加部分列の長さを `int` で返す | $$O(\log n)$$ |
| `range_lis_queries<Strict=true>(a,intervals,compare=std::less<>{})` | 各区間の答えを入力順の `std::vector<int>` で返す | $$O(n\log^2 n+q\log n)$$ |
| `range_lis_queries_sqrt<Strict=true>(a,intervals,compare=std::less<>{})` | 平方分割を使い、各区間の答えを入力順の `std::vector<int>` で返す | $$O(n\sqrt{n}\log n+q\log n)$$ |
| `size()` / `empty()` | 列の長さ / 空列かどうか | $$O(1)$$ |
| `clear()` | 空列に戻す | $$O(n)$$ |
| `swap(other)` | 同じ `Strict` の構造と内容を交換する | $$O(1)$$ |

`a` には const でサイズ取得とランダムアクセスができる範囲を渡します。`vector`・`array`・C 配列・`span`・`deque`・`string_view` などを利用できます。要素のコピーや値の加減算は不要で、比較できれば使えます。元の列を変更・破棄しても、構築済みの答えには影響しません。

区間は `0 <= l <= r <= size()`、列の長さは `INT_MAX` 以下とします。空の列や空区間の答えは 0 です。部分列そのものの復元や、構築後の要素更新には対応しません。列全体の長さや復元だけが必要な場合は [LIS](lis.md) を利用できます。

全ての問い合わせ区間を先に用意できる場合は、一括関数を使えます。`intervals` は `std::span<const std::pair<int,int>>` で、`std::vector<std::pair<int,int>>` もそのまま渡せます。各組は半開区間の `l,r` です。

```cpp
#include "lib/data_structure/range_lis_query.hpp"
#include <cassert>
#include <utility>
#include <vector>

int main() {
	std::vector<int> a{3, 1, 2, 2, 4};
	std::vector<std::pair<int, int>> intervals{ {0, 5}, {2, 2}, {1, 4}, {0, 5} };
	assert((range_lis_queries(a, intervals) == std::vector<int>{3, 0, 2, 3}));
	assert((range_lis_queries<false>(a, intervals) == std::vector<int>{4, 0, 3, 4}));
	assert((range_lis_queries_sqrt(a, intervals) == std::vector<int>{3, 0, 2, 3}));
}
```

2つの一括関数は同じ引数・返り値を持ち、`Strict` と比較関数の意味はクラスと同じです。区間の順序や重複は任意で、空区間には0を返します。入力列・区間列は変更せず、返り値は入力を参照しません。列と区間列の長さはそれぞれ `INT_MAX` 以下とし、各区間は `0 <= l <= r <= n` を満たす必要があります。問い合わせが空なら空配列を返し、空列には `[0,0)` の問い合わせだけを渡せます。

参考: [平方分割版の実装](https://judge.yosupo.jp/submission/331206)、[Library Checker: Static Range LIS Query](https://judge.yosupo.jp/problem/static_range_lis_query)、[Tiskin: Semi-local string comparison](https://arxiv.org/abs/0707.3619)。

列の長さを $$n$$ とし、比較を定数時間とすると、前計算は $$O(n\log^2 n)$$ 時間、問い合わせは $$O(\log n)$$ 時間・$$O(1)$$ 追加空間です。保持空間は $$O(n\log n)$$ bit、前計算の作業領域は $$O(n)$$ 個の固定長整数です。

`range_lis_queries` は問い合わせ数を $$q$$ として $$O(n\log^2 n+q\log n)$$ 時間・$$O(n+q)$$ 作業空間を使い、結果の格納に $$O(q)$$ 空間を使います。`range_lis_queries_sqrt` は $$O(n\sqrt{n}\log n+q\log n)$$ 時間・$$O(n+q)$$ 作業空間を使います。後者は値の順位を長さ約 $$\sqrt{n}$$ の区画に分けて処理します。列が大きくなると、前者の時間計算量が有利になります。どちらも問い合わせが空なら $$O(1)$$、列の長さが1以下なら $$O(q)$$ 時間です。
