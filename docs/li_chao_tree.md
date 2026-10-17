---
title: "Li Chao Tree"
documentation_of: //data_structure/li_chao_tree.hpp
---

```cpp
#include "lib/data_structure/li_chao_tree.hpp"

LiChaoTree<> tree({-3, 0, 5});
tree.add_line(2, 1);
tree.add_segment(-3, 5, -1, 4);
auto value = tree.query(0);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `LiChaoTree(coordinates = {})` | 問い合わせる座標列を登録して構築。ソート・重複除去を行う |
| `build(coordinates)` | 座標列を登録し直す。既存の直線・線分は削除 |
| `add_line(a,b)` | 全登録座標に直線 $$ax+b$$ を追加 |
| `add_segment(l,r,a,b)` | 座標区間 $$[l,r)$$ に直線 $$ax+b$$ を追加。端点自体の登録は不要 |
| `query(x)` | 登録座標 `x` での最小値。対象がなければ `nullopt` |
| `index(x)` / `coordinate(i)` | 登録座標の 0 始まり添字／添字に対応する座標 |
| `add_segment_index(l,r,a,b)` | ソート・重複除去後の添字区間 $$[l,r)$$ に直線を追加 |
| `query_index(i)` | 登録座標 `coordinate(i)` での最小値。対象がなければ `nullopt` |
| `size()` | 重複除去後の登録座標数 |
| `clear()` | 座標列を残して全ての直線・線分を削除 |

クエリ座標は構築時に全て登録します。`query(x)`・`index(x)` には登録済みの座標を渡します。添字はソート・重複除去後の 0 始まりです。

`LiChaoTree<T, true>` では `query`・`query_index` が最大値を返します。既定の `T` は `std::int64_t`、整数の計算・返り値は 32 bit 型なら `std::int64_t`、64 bit 型なら `__int128_t` です。第 3 テンプレート引数で計算型を指定できます。

登録時の座標数を $$n$$ として、構築は $$O(n \log n)$$（ソート済みなら $$O(n)$$）、直線追加・問い合わせ・`index` は $$O(\log n)$$、線分追加は $$O(\log^{2} n)$$、`clear` は $$O(n)$$ です。空間は $$O(n)$$ です。
