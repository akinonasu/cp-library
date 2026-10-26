---
title: "Monge グラフの d 辺最短路"
documentation_of: //graph/monge_d_edge_shortest_path.hpp
library_category: graph/shortest_path
---

頂点番号が増える向きに全ての辺を持つ Monge DAG で、始点から終点までちょうど指定本数の辺を使う最短路長を求めます。負の辺コストも扱えます。辺は列挙せず、コストを返す関数で渡します。

```cpp
#include "lib/graph/monge_d_edge_shortest_path.hpp"
#include <cassert>

auto weight = [](int i, int j) { return 1LL * (j - i) * (j - i); };
auto answer = monge_d_edge_shortest_path(6, 2, weight);
assert(answer && *answer == 13);
assert(!monge_d_edge_shortest_path(6, 0, weight));
assert(monge_d_edge_shortest_path(1, 0, weight) == 0);
```

`monge_d_edge_shortest_path<Distance = std::int64_t>(n, d, weight)` は `std::optional<Distance>` を返します。`n` は頂点数、`d` は使用する辺数で、どちらも非負の `int` です。頂点は `[0,n)`、始点は `0`、終点は `n-1` です。経路長だけを返し、経路の復元は行いません。

`weight(i,j)` は `0 <= i < j < n` の全てについて有限な辺コストを返す必要があります。自己辺・逆向きの辺・欠けた辺は扱いません。コールバックは符号付き 64 bit 以下の整数型を返し、同じ引数には同じ値を返してください。

全ての添字 $$0\le a<b<c<e<n$$ で、次の Monge 不等式が必要です。条件の検査は行いません。

$$
w(a,c)+w(b,e)\le w(a,e)+w(b,c).
$$

辺数と空入力の扱いは次のとおりです。

| 入力 | 返り値 |
| --- | --- |
| `n == 0` | `std::nullopt` |
| `n == 1 && d == 0` | `0` |
| `n == 1 && d > 0` | `std::nullopt` |
| `n > 1 && (d == 0 || d >= n)` | `std::nullopt` |
| `n > 1 && 1 <= d && d < n` | ちょうど `d` 辺を使う最短路長 |

上の表の最後の行以外では `weight` を呼びません。辺コストや複数の経路が同じ値でも使用できます。

`Distance` は符号付き 128 bit 以下の整数型です。既定の 64 bit に収まらない答えが必要な場合は、`monge_d_edge_shortest_path<__int128_t>(n,d,weight)` を使えます。内部の距離・ペナルティ・比較は `__int128_t` で計算し、返り値が `Distance` の範囲外なら `std::overflow_error` を送出します。コールバック内の演算はこの拡張の対象外なので、`weight` 自体がオーバーフローしない型で計算してください。

ちょうど $$k$$ 辺の最短路長を $$F(k)$$ とすると、Monge 性により $$F$$ は離散凸です。各辺に整数ペナルティ $$\lambda$$ を加えた最短路を計算し、同じ長さでは辺数が少ない経路を選んで、必要な傾きを二分探索します。最適辺数が飛ぶ場合も、同じ傾き上にある $$F(d)$$ を求められます。

探索区間は次の値から自動で計算するので、コストの上限やペナルティの範囲を指定する必要はありません。

$$
F(n-2)-F(n-1)\le\lambda\le F(1)-F(2).
$$

ここで $$F(1)=w(0,n-1)$$、$$F(2)=\displaystyle\min_{1\le i<n-1}(w(0,i)+w(i,n-1))$$ です。$$F(n-1)$$ は隣接する全ての辺の和、$$F(n-2)$$ はその経路から内部頂点を 1 個除いたときの最小値で、それぞれ線形時間で求められます。ペナルティを固定した最短路計算には [Online Monge Minima](https://qandori.github.io/cp-library/components/online_monge_minima.hpp.html) を使います。

参考: [Nyaan's Library](https://github.com/NyaanNyaan/library/blob/master/dp/monge-d-edge-shortest-path.hpp)、[maspypy library](https://github.com/maspypy/library/blob/main/convex/monge/monge_shortest_path_d_edge.hpp)。

`weight` の評価を $$O(1)$$、全ての辺について $$\lvert w(i,j)\rvert\le W$$、$$W\ge2$$ とすると、時間は $$O(n\log n\log W)$$、追加空間は $$O(n)$$ です。ペナルティの探索幅を $$R$$ とすると、$$R\ge2$$ で時間は $$O(n\log n\log R)$$、幅が 0 または 1 の場合は $$O(n\log n)$$ です。`d == 1` は $$O(1)$$ 時間・空間、`d == n-1`・`d == 2`・`d == n-2` は $$O(n)$$ 時間・空間で直接計算します。コールバックを呼ばない入力は $$O(1)$$ 時間・空間です。
