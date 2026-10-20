---
title: "Li Chao Tree (クエリ先読み)"
documentation_of: //data_structure/li_chao_tree.hpp
---

```cpp
#include "lib/data_structure/li_chao_tree.hpp"

LiChaoTree<> tree({-3, 0, 5});
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
| `LiChaoTree(coordinates = {})` | クエリる座標列を登録して構築。ソート・重複除去を行う | $$O(n\log n)$$、ソート済み: $$O(n)$$ |
| `build(coordinates)` | 座標列を登録し直す。既存の直線・線分は削除 | $$O(n\log n)$$、ソート済み: $$O(n)$$ |
| `add_line(a,b)` | 全登録座標に直線 $$ax+b$$ を追加 | $$O(\log n)$$ |
| `add_segment(l,r,a,b)` | 座標区間 $$[l,r)$$ に直線 $$ax+b$$ を追加。端点自体の登録は不要 | $$O(\log^2 n)$$ |
| `query(x)` | 登録座標 `x` での最小値。対象がなければ `nullopt` | $$O(\log n)$$ |
| `index(x)` / `coordinate(i)` | 登録座標の 0 始まり添字／添字に対応する座標 | index: $$O(\log n)$$、coordinate: $$O(1)$$ |
| `add_segment_index(l,r,a,b)` | ソート・重複除去後の添字区間 $$[l,r)$$ に直線を追加 | $$O(\log^2 n)$$ |
| `query_index(i)` | 登録座標 `coordinate(i)` での最小値。対象がなければ `nullopt` | $$O(\log n)$$ |
| `size()` | 重複除去後の登録座標数 | $$O(1)$$ |
| `clear()` | 座標列を残して全ての直線・線分を削除 | $$O(n)$$ |

構築前にクエリを先読みし、クエリる座標を全て登録します。座標の候補が最初から分かっていれば、その候補を登録するだけで使えます。追加する直線・線分の先読みは不要です。`query(x)`・`index(x)` には登録済みの座標を渡します。添字はソート・重複除去後の 0 始まりです。クエリ座標を事前に集められない場合は [Dynamic Li Chao Tree](https://qandori.github.io/cp-library/data_structure/dynamic_li_chao_tree.hpp.html) を使えます。

`LiChaoTree<T, true>` では `query`・`query_index` が最大値を返します。既定の `T` は `std::int64_t`、整数の計算・返り値は 32 bit 型なら `std::int64_t`、64 bit 型なら `__int128_t` です。第 3 テンプレート引数で計算型を指定できます。

登録時の座標数を $$n$$ として、構築は $$O(n \log n)$$（ソート済みなら $$O(n)$$）、直線追加・クエリ・`index` は $$O(\log n)$$、線分追加は $$O(\log^{2} n)$$、`clear` は $$O(n)$$ です。空間は $$O(n)$$ です。
