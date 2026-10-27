---
title: K Shortest Walk
documentation_of: //graph/k_shortest_walk.hpp
library_category: graph/shortest_path
---

非負整数重みの有向グラフで、始点から終点への walk の長さを短い順に最大 `k` 個返します。頂点・辺を何度でも通れます。同じ長さの異なる walk は別々に数えます。

```cpp
#include "lib/graph/k_shortest_walk.hpp"
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main() {
	std::vector<std::vector<std::pair<int, int>>> g(4);
	g[0].emplace_back(1, 2);
	g[1].emplace_back(3, 3);
	g[0].emplace_back(3, 8);
	g[1].emplace_back(1, 2);
	auto lengths = k_shortest_walk(g, 0, 3, 5);
	assert((lengths == std::vector<std::int64_t>{5, 7, 8, 9, 11}));
	auto below = k_shortest_walk(g, 0, 3, 5, std::int64_t(8));
	assert((below == std::vector<std::int64_t>{5, 7}));
}
```

`k_shortest_walk<Distance = std::int64_t>(g, start, target, k, inf = std::numeric_limits<Distance>::max())` は `std::vector<Distance>` を返します。引数の順序と条件は次のとおりです。

| 引数 | 型・意味 |
| --- | --- |
| `g` | `std::vector<std::vector<std::pair<int, W>>>` または `DirectedCSR<W>`。辺は行き先と非負整数重み |
| `start` | `int`。始点の頂点番号 |
| `target` | `int`。終点の頂点番号 |
| `k` | 非負の `int`。取得する walk の最大個数 |
| `inf` | 正の `Distance`。取得する長さの上限。この値自体は含まない |

頂点数を `n` とすると、頂点番号と辺の行き先は `[0,n)` です。`k > 0` では `1 <= n <= INT_MAX` と有効な `start`・`target` が必要です。`k == 0` では頂点にアクセスせず空の列を返すため、空グラフでも使えます。グラフは変更しません。負の `k`・非正の `inf`・範囲外の頂点番号など、引数条件に違反する呼び出しは扱いません。

返り値は非減少順で、添字 `i` が 0 始まりの第 `i` 番目に短い walk の長さです。長さが `inf` 未満の walk が `k` 個に満たなければ、存在するものだけを返します。到達不能なら空の列です。長さの重複を取り除かず、不足分を特別な値で埋めることもありません。頂点列・辺列の復元は行いません。

walk は通った辺の列で区別します。同じ始点・終点・重みを持つ並行辺も、それぞれ別の辺です。自己ループと重み 0 の閉路も扱えます。重み 0 の閉路により同じ長さの walk が無限に存在する場合も、要求した個数まで返します。

`start == target` では、辺を使わない長さ 0 の walk を含めます。途中で終点に着いてから別の辺を通り、再び終点に戻る walk も数えます。頂点の再訪を禁じる単純路の列挙には使えません。

`W` と `Distance` は `bool` を除く符号付き・符号なしの 128 bit 以下の整数型です。負の辺重みは扱いません。隣接リスト・CSR のどちらでも、`inf` を省略したときの距離型は既定で `std::int64_t` です。`k_shortest_walk<__int128_t>(g,start,target,k)` のように明示するか、希望する型の `inf` を渡すと距離型を選べます。浮動小数点・多倍長整数型には対応しません。

`inf` 以上の長さは返さず、距離の加算前に上限を確認します。辺重みが距離型に収まらない場合や、長い walk が距離型の上限を超える場合も、整数オーバーフローを起こさず除外します。既定の `inf` は距離型の最大値なので、その最大値に等しい答えが必要なら、より広い距離型を使ってください。

内部ヒープのノード番号は `int` です。永続化の複製を含むノード数が `INT_MAX - 1` を超える場合は `std::length_error` を送出します。

CSR を使う例です。32 bit の辺重みでも、長さは既定の 64 bit で計算します。

```cpp
#include "lib/graph/k_shortest_walk.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
	using Graph = DirectedCSR<std::uint32_t>;
	std::vector<Graph::Edge> edges{ {0, 1, 3000000000U}, {1, 2, 3000000000U}, {1, 1, 1U} };
	Graph g(3, edges);
	auto lengths = k_shortest_walk(g, 0, 2, 2);
	assert((lengths == std::vector<std::int64_t>{6000000000LL, 6000000001LL}));
}
```

Eppstein 法を使います。逆向きのグラフで終点への最短距離と最短路木を求め、最短路木から外れる辺による増分をヒープに保持します。各頂点のヒープをまとめてから、最短路木に沿うヒープを永続的に共有し、候補の walk を長さ順に取り出します。最短路木の構築・処理は再帰を使いません。

参考: David Eppstein, [Finding the k Shortest Paths](https://doi.org/10.1137/S0097539795290477), SIAM Journal on Computing, 1998.

頂点数を $$n$$、辺数を $$m$$、要求する個数を $$k$$ とします。64 bit 以下の距離型では radix heap を使い、そのビット数を $$w\le64$$ として、時間は $$O(n+m+n\log m+(m+k)w)$$ です。128 bit の距離型では比較ヒープを使い、時間は $$O(n+m\log m+n\log m+k\log k)$$ です。追加空間はどちらも $$O(n+m+n\log m+k)$$ です。整数の演算・比較・コピーは定数時間とします。対数に対応する処理で要素が 0 個または 1 個の場合は定数時間です。`k == 0` は $$O(1)$$ 時間・空間です。
