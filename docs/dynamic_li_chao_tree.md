---
title: "Dynamic Li Chao Tree"
documentation_of: //data_structure/dynamic_li_chao_tree.hpp
---

```cpp
#include "lib/data_structure/dynamic_li_chao_tree.hpp"

DynamicLiChaoTree<> tree(-1000000000LL, 1000000001LL);
tree.add_line(2, 1);
tree.add_segment(-3, 5, -1, 4);
auto value = tree.query(0);
if (value) {
	long long answer = static_cast<long long>(*value); // 1
}
```

例では直線 $$2x+1$$ と、$$-3\le x<5$$ にだけ有効な直線 $$-x+4$$ を追加しています。`query(0)` の答えは 1 です。返り値は `std::optional` なので、値があることを確認して `*value` で取り出します。対象の直線がない場合は `nullopt` です。例の整数への変換は、答えが `long long` に収まる場合に使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DynamicLiChaoTree(lower,upper)` | 整数座標の範囲 `[lower,upper)` を指定。省略時は `T` の全範囲 | $$O(1)$$ |
| `build(lower,upper)` | 座標範囲を変更し、全ての直線・線分を削除 | $$O(N)$$ |
| `lower()` / `upper()` | 座標範囲の左端／右端を `bound_type` で返す | $$O(1)$$ |
| `add_line(a,b)` | 座標範囲全体に直線 $$ax+b$$ を追加 | 償却 $$O(\log C)$$ |
| `add_segment(l,r,a,b)` | 半開区間 $$[l,r)$$ に直線 $$ax+b$$ を追加。座標範囲外は切り詰め、空区間は無視 | 償却 $$O(\log^2 C)$$ |
| `query(x)` | 座標 `x` での最小値を `optional<value_type>` で返す。対象がなければ `nullopt` | $$O(\log C)$$ |
| `empty()` | 直線・線分を含まないか | $$O(1)$$ |
| `node_count()` | 使用中のノード数。線分追加用の中間ノードも含む | $$O(1)$$ |
| `reserve(count)` | 指定数のノード領域を事前確保。直線・線分は変更しない | $$O(N)$$ |
| `clear()` | 座標範囲と確保領域を残して全ての直線・線分を削除 | $$O(N)$$ |
| `value_type` / `bound_type` | 評価値の型／区間端点の型 | —（型） |

クエリ座標の事前登録は不要で、追加・クエリをオンラインで処理します。`query(x)` の `x` は指定した座標範囲内である必要があります。直線の削除は行いません。座標を全て先読みできる場合は [Li Chao Tree (クエリ先読み)](https://qandori.github.io/cp-library/data_structure/li_chao_tree.hpp.html) も使えます。

`DynamicLiChaoTree<T, true>` では最大値を返します。`T` は座標と係数に使う 64 bit 以下の符号付き整数型で、既定は `std::int64_t` です。浮動小数点座標は扱いません。

既定の `value_type` は、`T` が 32 bit 以下なら `std::int64_t`、64 bit なら `__int128_t` です。第 3 テンプレート引数で計算・返り値の型を指定できます。狭い型を指定する場合、全ての有効な座標で積と和がその型に収まる必要があります。既定の 128 bit 評価でも、追加した直線・線分の有効範囲で値が 64 bit に収まる間は軽い比較を使います。

`bound_type` は `T` が 32 bit 以下なら `std::int64_t`、64 bit なら `__int128_t` です。最大座標の一つ先を端点に指定できるため、既定の範囲は `T` の最大値も含みます。区間は全て右端を含みません。

```cpp
DynamicLiChaoTree<> all;
using B = decltype(all)::bound_type;
auto last = std::numeric_limits<std::int64_t>::max();
all.add_segment(B(last), B(last) + 1, 0, 7);
auto value = all.query(last);
```

必要なノードだけを連続配列に確保します。通常の直線追加では新規ノードは高々 1 個です。座標範囲が分かる場合は狭く指定すると、走査の深さを抑えられます。

座標範囲の整数の個数を $$C$$、直線・線分の追加回数をそれぞれ $$L,S$$、使用中のノード数を $$N$$ とします。直線追加は償却 $$O(\log C)$$、線分追加は償却 $$O(\log^2 C)$$、クエリは $$O(\log C)$$、ノードの空間は $$O(L+S\log C)$$ です。構築は $$O(1)$$、`build`・`clear`・確保領域を増やす `reserve` は $$O(N)$$、その他のクエリは $$O(1)$$ です。
