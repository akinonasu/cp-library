---
title: Dynamic Segtree
documentation_of: //data_structure/dynamic_segtree.hpp
redirect_from:
- /data_structure/segment_tree/dynamic_segtree.hpp.html
- /lightweight/data_structure/segment_tree/dynamic_segtree.hpp.html
library_category: data_structure/segment_tree
---

```cpp
#include "lib/data_structure/dynamic_segtree.hpp"

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }

DynamicSegtree<long long, op, e> seg(1LL << 60);
seg.reserve(200000);
seg.set(123, 10);
seg.multiply(123, 5);
auto x = seg.get(123);
auto sum = seg.prod(0, 1000);
auto r = seg.max_right(0, [](long long s) { return s <= 10; });
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DynamicSegtree(n = 0)` / `DynamicSegtree(l,r)` | $$[0,n)$$／$$[l,r)$$ を単位元で初期化 | $$O(1)$$ |
| `set(p,x)` | 座標 `p` に代入 | 償却 $$O(\min(B,k))$$ |
| `multiply(p,x)` | `op(現在値,x)` に更新 | 償却 $$O(\min(B,k))$$ |
| `get(p)` | 1 点の値。未更新なら `e()` | $$O(\min(B,k))$$ |
| `prod(l,r)` / `all_prod()` | $$[l,r)$$／全体の集約 | prod: $$O(\min(B,k))$$、all_prod: $$O(1)$$ |
| `max_right(l,f)` / `min_left(r,f)` | 区間積が `f` を満たす最大の右端／最小の左端 | $$O(\min(B,k))$$ |
| `size()` / `left_bound()` / `right_bound()` | 座標範囲の長さ／左端／右端 | $$O(1)$$ |
| `node_count()` | 更新済みの異なる座標数 | $$O(1)$$ |
| `reserve(k)` / `clear()` | 領域予約／容量を残して全座標を単位元に戻す | $$O(k)$$ |
| `memory_usage()` | 確保済みノード領域のバイト数 | $$O(1)$$ |
| `swap(other)` | 内容と座標範囲を交換 | $$O(1)$$ |

`op` は結合的な演算、`e()` は単位元です。非可換な演算にも対応します。未更新の座標の値は `e()` です。

構築範囲は `DynamicSegtree(n)` が $$[0,n)$$、`DynamicSegtree(l,r)` が $$[l,r)$$ です。座標型は第 4 テンプレート引数で指定でき、既定は `std::int64_t` です。

`set(p,x)` は代入、`multiply(p,x)` は現在値を `op(現在値,x)` に更新します。`prod(l,r)` は $$[l,r)$$ の積、`all_prod()` は全体の積です。

`max_right(l,f)`・`min_left(r,f)` は区間積が `f` を満たす境界を返します。`f(e())` が真で、区間を伸ばすと真から偽へ単調に変化する必要があります。

`reserve(k)` は更新する異なる座標数を目安に領域を予約します。`node_count()` はその個数、`memory_usage()` は確保済みノード領域のバイト数です。更新する異なる座標数の上限は $$2^{26}-1$$ です。`clear()` は確保領域を残して初期状態に戻します。

座標型の bit 数を $$B$$、更新済み座標数を $$k$$ として、更新は償却 $$O(\min(B,k))$$、`get`・`prod`・境界探索は $$O(\min(B,k))$$ です。構築・`all_prod`・サイズ取得は $$O(1)$$、`reserve`・`clear` は $$O(k)$$ です。空間は $$O(k)$$（予約容量が大きい場合はその容量分）です。
