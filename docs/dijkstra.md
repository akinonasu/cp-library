---
title: Dijkstra
documentation_of: //graph/dijkstra.hpp
redirect_from:
- /graph/shortest_path/dijkstra.hpp.html
- /lightweight/graph/shortest_path/dijkstra.hpp.html
library_category: graph/shortest_path
---

非負重みのグラフで、始点からの最短距離や経路を求めます。隣接リストのほか、隣接辺を 1 本の配列にまとめて保存する [DirectedCSR](directed_csr.md) を渡せます。以下の DAG は、有向閉路のないグラフを指します。

```cpp
#include "lib/graph/dijkstra.hpp"

std::vector<std::vector<std::pair<int, std::int64_t>>> g(3);
g[0].emplace_back(1, 5);
auto dist = dijkstra(g, 0); // {0,5,-1}
auto [distance, path] = dijkstra_path(g, 0, 1); // 5, {0,1}
```

頂点数を $$n$$、辺数を $$m$$ とします。計算量は辺重みの型ではなく、計算に使う距離型で決まります。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `dijkstra(g,start)` / `dijkstra(g,start,inf)` | 非負重みのグラフから始点の最短距離を `std::vector` で返す。要素型は距離型で、到達不能は -1 | 64 bit 以下の整数距離: $$O(n+m)$$、その他: $$O(n+m\log m)$$ |
| `dijkstra_path(g,start,target)` / `dijkstra_path(g,start,target,inf)` | 距離型と `std::vector<int>` の `std::pair`。頂点列は始点・終点を含み、到達不能なら距離 -1、空列 | CSR で各入次数が高々 1: $$O(n+m)$$、64 bit 以下の整数距離: $$O(n+m)$$、その他: $$O(n+m\log m)$$ |
| `dijkstra<true>(g,start,inf)` | 入次数 0 の部分を先に処理し、残りを Dijkstra 法で求める。`inf` は省略可 | DAG: $$O(n+m)$$、一般の場合は通常版と同じ |
| `dijkstra_path<true>(g,start,target,inf)` | 同じ前処理を使う距離・経路の取得。`inf` は省略可 | DAG: $$O(n+m)$$、一般の場合は通常版と同じ |

辺は `{行き先, 重み}`、頂点番号は 0 始まりです。重みは非負とします。

全頂点への距離が必要なら `dijkstra`、一つの目的地までの経路が必要なら `dijkstra_path` を使います。例では頂点 0 から 1 までの距離が 5、経路が `{0,1}` です。`start==target` の経路は `{start}` です。並行辺がある場合も頂点列を返し、辺 ID は返しません。

CSR では、各頂点の入次数が高々 1 なら、一意な経路の探索を自動で選びます。出辺が多数ある木や、閉路から枝が伸びたグラフでも使えます。距離順に並べるキューと全頂点の距離配列を省き、目的地を見つけた時点で返します。[内部処理の説明](https://qandori.github.io/cp-library/components/dijkstra_unique.hpp.html)もあります。

`<true>` を付けると、入次数 0 の頂点から距離を確定し、残った部分だけを Dijkstra 法で探索します。有向閉路があっても使え、DAG なら線形時間です。グラフ全体の入次数を数えるので、無向グラフや始点から近い範囲だけを調べたい場合は通常版が適しています。省略時は前処理を行いません。[前処理の説明](https://qandori.github.io/cp-library/components/dijkstra_topological.hpp.html)もあります。

静的な大きいグラフでは、[Directed CSR](https://qandori.github.io/cp-library/components/directed_csr.hpp.html) も同じ関数へ渡せます。各頂点の隣接辺を一つの配列へまとめ、頂点ごとの領域確保を省きます。

```cpp
#include "lib/graph/dijkstra.hpp"

using Graph = DirectedCSR<std::uint32_t>;
std::vector<Graph::Edge> edges{ {0,1,3000000000U}, {1,2,3000000000U} };
Graph graph(3, edges);
auto [cost, vertices] = dijkstra_path<true>(graph, 0, 2);
// cost = 6000000000, vertices = {0,1,2}
```

隣接リスト・CSR ともに、明示した `inf` の型を距離型として使います。隣接リストで `inf` を省略すると、重みと同じ型です。CSR で省略すると、32 bit 以下の整数重みでは `std::int64_t`、64 bit 整数・浮動小数点などでは重みと同じ型です。辺重みは元の型のまま保存でき、距離型に合わせてグラフを作り直す必要はありません。

例えば、辺を `std::uint32_t` で保存し、距離は `std::int64_t` で計算できます。

```cpp
#include "lib/graph/dijkstra.hpp"

std::vector<std::vector<std::pair<int, std::uint32_t>>> graph(3);
graph[0].emplace_back(1, 3000000000U);
graph[1].emplace_back(2, 3000000000U);
auto distances = dijkstra(graph, 0, INT64_MAX); // {0,3000000000,6000000000}
auto [cost, path] = dijkstra_path<true>(graph, 0, 2, INT64_MAX);
// cost = 6000000000, path = {0,1,2}
```

距離型は必要な辺重み・距離を扱えるものにしてください。浮動小数点重みでは同じ型または精度の高い浮動小数点型の `inf` を渡せます（`float` の辺に `100.0` を渡して `double` 距離を使う、など）。小数の重みを整数距離に変換する指定には対応しません。CSR は構築後に辺を変えず、`build` で再構築します。

入力から直接構築する場合は、CSR のコールバックと[一括入力](https://qandori.github.io/cp-library/components/fast_io_bulk.hpp.html)を組み合わせられます。次の例では `m` 本の辺を入力順に読みます。各行の形式は `始点 終点 重み` です。

```cpp
#include "lib/graph/dijkstra.hpp"
#include "lib/components/fast_io_bulk.hpp"

FastInput in;
int n, m, s, t;
in.read(n); in.read(m); in.read(s); in.read(t);
using Graph = DirectedCSR<std::uint32_t>;
Graph g(n, m, [&](auto emit) {
	read_unsigned_records<3>(in, m, [&](auto from, auto to, auto weight) {
		emit(Graph::Edge{int(from), int(to), weight});
	});
});
auto [distance, path] = dijkstra_path(g, s, t);
```

整数は `uint32_t` の範囲とし、頂点番号は `int` の範囲に収めてください。

`dijkstra(g, start, inf)` は始点からの距離を返し、到達不能は `-1` です。`inf` は扱う距離より大きい値を指定します。既定値は $$\min(4\cdot 10^{18},\text{距離型の最大値})$$ です。

距離が `inf` 以上となる経路は扱いません。符号なし整数型での到達不能値は、その型へ変換した `-1` です。整数の加算前に上限を確認し、オーバーフローを避けます。無制限の多倍長整数型の既定 `inf` は $$4\cdot 10^{18}$$ なので、より大きい答えには明示してください。

64 bit 以下の整数距離では $$O(n+m)$$、その他では $$O(n+m\log m)$$ 時間です。追加空間は $$O(n+m)$$ です。固定幅整数の演算は定数時間として扱います。

`<true>` の前処理は $$O(n+m)$$ 時間・$$O(n)$$ 追加空間です。DAG では探索全体が $$O(n+m)$$ 時間になります。`start==target` の `dijkstra_path` は $$O(1)$$ 時間・空間です。要素の演算・比較・コピーを $$O(1)$$ とします。

CSR で各頂点の入次数が高々 1 の場合、`dijkstra_path` は $$O(n+m)$$ 時間・追加空間です。始点に出辺がない場合は $$O(1)$$ 時間・空間です。
