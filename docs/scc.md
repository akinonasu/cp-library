---
title: 強連結成分分解
documentation_of: //graph/scc.hpp
library_category: graph/connectivity
redirect_from:
- /graph/connectivity/scc.hpp.html
- /lightweight/graph/connectivity/scc.hpp.html
---

```cpp
#include "lib/graph/scc.hpp"

SCCGraph graph(4);
graph.add_edge(0, 1);
graph.add_edge(1, 0);
graph.add_edge(1, 2);
auto groups = graph.scc();
auto [count, id] = graph.scc_ids();
auto components = graph.decomposition();
for (int c = 0; c < components.size(); ++c) {
	for (int v : components[c]) {
		// 成分 c の頂点
	}
}
```

互いに到達できる頂点を同じ組に分けます。この組を強連結成分と呼びます。

`SCCGraph(n)` で 0 始まりの `n` 頂点を作り、`add_edge(from,to)` で有向辺を追加します。自己ループ・多重辺・空グラフにも対応します。

`scc()` は成分の配列を返し、各成分内の頂点は昇順です。成分は、異なる成分を結ぶ辺が必ず前から後へ向かう順に並びます。独立した成分間の順序は保証しません。

`scc_ids()` は成分数と長さ `n` の成分 ID 配列を返します。同じ成分の頂点は同じ ID を持ち、ID は `scc()` の成分番号と一致します。結果を保持しないため、呼ぶたびに再計算します。辺追加後も再利用でき、コピーは独立します。

`decomposition()` は次の `Components` を返します。成分の順序・各成分の頂点順は `scc()` と同じです。

| メンバー | 内容 |
| --- | --- |
| `size()` | 成分数 |
| `id[v]` | 頂点 `v` の成分 ID |
| `operator[](c)` | 成分 `c` の頂点列を `span<const int>` で取得 |
| `vertices` | 全成分の頂点列を連結した配列 |
| `offset` | 成分 `c` が `vertices` の半開区間 `[offset[c],offset[c+1])` に対応する境界配列 |

結果はグラフを参照せず、コピーは独立します。頂点列の `span` は、その結果の破棄・代入や配列の変更まで有効です。各メンバーの参照・頂点列の取得は $$O(1)$$ です。

`SCCGraph::size()` は頂点数、`edge_count()` は辺数を、それぞれ $$O(1)$$ 時間で返します。`reserve(r)` は辺の格納容量を予約し、予約量 `r` は `std::size_t` で `INT_MAX` 以下にします。頂点数は非負の `int`、辺数は `INT_MAX` 以下、辺の端点は `[0,n)` が必要です。

頂点数を $$n$$、辺数を $$m$$ として、構築は $$O(1)$$、辺追加は償却 $$O(1)$$、分解は $$O(n+m)$$ 時間・$$O(n+m)$$ 追加空間、グラフの保持は $$O(n+m)$$ 空間です。`reserve(r)` は再確保がある場合に $$O(m)$$ 時間、ない場合に $$O(1)$$ 時間で、予約した容量 $$O(r)$$ も保持空間に含めます。
