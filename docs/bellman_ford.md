---
title: Bellman-Ford
documentation_of: //graph/bellman_ford.hpp
---

負の重みを持つ辺を含む有向グラフの単一始点最短路を求めます。経路と、始点から到達できる負閉路を復元できます。自己辺・多重辺に対応します。

```cpp
#include "lib/graph/bellman_ford.hpp"

int main() {
	std::vector<DirectedCSR<int>::Edge> edges{ {0, 1, 4}, {0, 2, 5}, {1, 2, -7}, {2, 3, 2} };
	DirectedCSR<int> g(5, edges);
	auto result = bellman_ford(g, 0);
	if (result.has_negative_cycle()) return 0;
	assert(result.distance[2] == -3);
	assert(result.distance[3] == -1);
	assert(!result.reachable(4));
	auto vertices = result.path(3);       // 0, 1, 2, 3
	auto edges_in_path = result.path_edges(3);
	assert(vertices.size() == 4 && edges_in_path.size() == 3);
}
```

グラフは `DirectedCSR<W>` または `vector<vector<pair<int,W>>>` を渡します。各辺は `(行き先,重み)` です。頂点番号は 0 始まりで、始点は存在する頂点を指定します。重みは符号付き整数型です。

距離型の既定は `std::int64_t`。`bellman_ford<__int128_t>(g,s)` のように指定もできます。距離型は重み型以上の桁数を持つ符号付き整数型を使ってください。加算途中も距離型に収まり、有限値がその型の最大値と一致しないことが必要です。範囲外の計算を検出した場合は `std::overflow_error` を送出します。値を丸めて計算を継続することはありません。

`distance[v]` が距離型の最大値なら到達不能です。負の距離、特に `-1` も正しい答えになり得るため、到達判定には `reachable(v)` を使います。始点自身の距離は 0、経路は `[s]` です。到達不能頂点への経路は空配列です。

`path_edges(t)[i]` は、`path(t)[i]` の **隣接リスト内の添字** です。入力辺の通し番号ではありません。`g[path[i]][edge_indices[i]]` が使用した辺になります。多重辺でも実際に選んだ辺を区別できます。`parent_edge[v]` も、`g[parent[v]]` 内の添字です。

到達可能な負閉路が見つかると、`has_negative_cycle()` が真になります。`negative_cycle` は先頭と末尾が同じ閉路の頂点列、`negative_cycle_edges` はその各辺の隣接リスト内の添字です。両者から復元した閉路の重みは負です。始点から到達不能な負閉路は報告しません。

負閉路が見つかった場合、`distance`・`parent` は計算途中の値です。最短路の取得には使わず、`reachable`・`path`・`path_edges` も呼ばないでください。負閉路から到達する全頂点を負無限大として列挙する機能ではありません。

`bellman_ford` は通常の緩和走査を最大 2 回行い、更新が止まれば結果を返します。続く場合は到達可能部分を強連結成分に分けます。成分間はトポロジカル順に処理し、成分内に負辺がなければ Dijkstra、負辺があれば Goldberg–Radzik 法を使います。DAG では負辺があっても線形時間です。重みが全て非負と分かっている場合は、通常の `dijkstra` も使えます。

`goldberg_radzik` は前処理を省き、Goldberg–Radzik 法だけで解きます。入出力の型と復元機能は同じです。既定の距離型ではどちらも一般の場合の最悪計算量は $$O(nm)$$ で、全ての疎グラフをほぼ線形時間で解く保証はありません。

頂点数を $$n$$、辺数を $$m$$、復元する頂点・辺の数を $$L$$ とします。

| 操作・メンバー | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `bellman_ford<Distance = std::int64_t>(g,s)` | 最短路と到達可能な負閉路の検出・復元 | 64 bit 以下は最悪 $$O(n+nm)$$、128 bit は $$O(n+nm+m\log m)$$。DAG は $$O(n+m)$$ |
| `goldberg_radzik<Distance = std::int64_t>(g,s)` | 成分分解と前段の走査を省いて同じ結果を求める | 最悪 $$O(n+nm)$$ |
| `result.has_negative_cycle()` | 到達可能な負閉路が見つかったか | $$O(1)$$ |
| `result.reachable(v)` | 負閉路がない場合、`v` に到達できるか | $$O(1)$$ |
| `result.distance[v]` | 始点からの距離。到達不能なら `infinity` | $$O(1)$$ |
| `result.parent[v]` | 最短路上の直前の頂点。始点は自身、到達不能は `-1` | $$O(1)$$ |
| `result.parent_edge[v]` | 直前の頂点の隣接リスト内の辺番号。始点・到達不能は `size_t(-1)` | $$O(1)$$ |
| `result.path(t)` / `result.path_edges(t)` | 最短路の頂点列 / 各辺の隣接リスト内の添字。到達不能なら空配列 | $$O(L)$$ |
| `result.negative_cycle` / `result.negative_cycle_edges` | 負閉路の頂点列 / 各辺の隣接リスト内の添字。負閉路がなければ空配列 | 参照 $$O(1)$$、走査 $$O(L)$$ |
| `result.source` / `result.infinity` | 指定した始点 / 到達不能を表す値 | $$O(1)$$ |

グラフを除く追加空間は、`bellman_ford` が $$O(n+m)$$、`goldberg_radzik` が $$O(n)$$ です。64 bit 以下の整数の非負辺成分では既存の radix heap を使い、128 bit の距離型では比較ヒープを使います。ヒープには更新前の古い候補も入るため、辺数に比例する領域を使う場合があります。再帰呼び出しを使わず、長い一本道にも対応します。
