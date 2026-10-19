---
title: "Dijkstra"
documentation_of: //graph/dijkstra.hpp
---

```cpp
#include "lib/graph/dijkstra.hpp"

std::vector<std::vector<std::pair<int, std::int64_t>>> g(3);
g[0].emplace_back(1, 5);
auto dist = dijkstra(g, 0); // {0,5,-1}
auto [distance, path] = dijkstra_path(g, 0, 1); // 5, {0,1}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `dijkstra(g,start)` / `dijkstra(g,start,inf)` | 非負重みの隣接リストから始点の最短距離列を返す。到達不能は -1 | 64 bit 以下の整数距離: $$O(n+mw)$$、その他: $$O(n+m\log m)$$ |
| `dijkstra_path(g,start,target)` / `dijkstra_path(g,start,target,inf)` | 目的地までの距離と、始点・終点を含む頂点列の `pair`。到達不能なら距離 -1、空の頂点列 | 64 bit 以下の整数距離: $$O(n+mw)$$、その他: $$O(n+m\log m)$$ |
| `dijkstra<true>(g,start,inf)` | 入次数 0 の部分を先に処理し、残りを Dijkstra 法で求める。`inf` は省略可 | DAG: $$O(n+m)$$、一般の場合は通常版と同じ |
| `dijkstra_path<true>(g,start,target,inf)` | 同じ前処理を使う距離・経路の取得。`inf` は省略可 | DAG: $$O(n+m)$$、一般の場合は通常版と同じ |

辺は `{行き先, 重み}`、頂点番号は 0 始まりです。重みは非負とします。

全頂点への距離が必要なら `dijkstra`、一つの目的地までの経路が必要なら `dijkstra_path` を使います。例では頂点 0 から 1 までの距離が 5、経路が `{0,1}` です。`start==target` の経路は `{start}` です。並行辺がある場合も頂点列を返し、辺 ID は返しません。

`dijkstra_path` は見つけた目的地への距離を上界とし、それより短い経路が残っていない時点で終了します。緩和時に親を保存するため、全頂点の探索や経路の再走査を省けます。

`<true>` を付けると、入次数 0 の頂点から距離を確定し、残った部分だけを Dijkstra 法で探索します。有向閉路があっても使え、DAG なら線形時間です。グラフ全体の入次数を数えるので、無向グラフや始点から近い範囲だけを調べたい場合は通常版が適しています。省略時は前処理を行いません。[前処理の説明](https://qandori.github.io/cp-library/components/dijkstra_topological.hpp.html)もあります。

静的な大きいグラフでは、[Directed CSR](https://qandori.github.io/cp-library/components/directed_csr.hpp.html) も同じ関数へ渡せます。各頂点の隣接辺を一つの配列へまとめ、頂点ごとの領域確保を省きます。

```cpp
using Graph = DirectedCSR<std::uint32_t>;
std::vector<Graph::Edge> edges{ {0,1,3000000000U}, {1,2,3000000000U} };
Graph graph(3, edges);
auto [cost, vertices] = dijkstra_path<true>(graph, 0, 2);
// cost = 6000000000, vertices = {0,1,2}
```

CSR の 32 bit 以下の整数重みは、距離を `std::int64_t` で計算します。64 bit 整数・浮動小数点などでは重みと同じ型です。明示した `inf` の型で距離型を指定することもできます。浮動小数点重みには同じ型の `inf` を渡します（`double` なら `100.0` など）。隣接リスト版では従来どおり重みと距離が同じ型です。CSR は構築後に辺を変えず、`build` で再構築します。

`dijkstra(g, start, inf)` は始点からの距離を返し、到達不能は `-1` です。`inf` は扱う距離より大きい値を指定します。既定値は $$\min(10^{18},\text{距離型の最大値})$$ です。

距離が `inf` 以上となる経路は扱いません。符号なし整数型での到達不能値は、その型へ変換した `-1` です。整数の加算前に上限を確認し、オーバーフローを避けます。浮動小数点・128 bit 整数・Boost の多倍長整数では比較ヒープを使います。無制限の多倍長整数型の既定 `inf` は $$10^{18}$$ なので、より大きい答えには明示してください。

整数のキューは、小さい固定配列と、距離の窓に応じた 32 bit／64 bit の radix heap を使います。距離の型や扱える範囲は変わりません。出辺のない頂点は距離だけ更新し、キューへの追加を省きます。

$$n$$ 頂点・$$m$$ 辺、距離型のビット数を $$w \le 64$$ として、64 bit 以下の整数距離では $$O(n + mw)$$、その他では $$O(n + m \log m)$$ 時間です。追加空間は $$O(n + m)$$ です（整数幅は固定）。

`<true>` の前処理は $$O(n+m)$$ 時間・$$O(n)$$ 追加空間です。DAG では探索全体が $$O(n+m)$$ 時間になります。`start==target` の `dijkstra_path` は $$O(1)$$ 時間・空間です。要素の演算・比較・コピーを $$O(1)$$ とします。
