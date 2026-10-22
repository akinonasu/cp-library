---
title: "強連結成分分解"
documentation_of: //graph/connectivity/scc.hpp
---

```cpp
#include "lib/graph/connectivity/scc.hpp"

SCCGraph graph(4);
graph.add_edge(0, 1);
graph.add_edge(1, 0);
graph.add_edge(1, 2);
auto groups = graph.scc();
auto [count, id] = graph.scc_ids();
```

`SCCGraph(n)` で 0 始まりの `n` 頂点を作り、`add_edge(from,to)` で有向辺を追加します。自己ループ・多重辺・空グラフにも対応します。

`scc()` は成分の配列を返し、各成分内の頂点は昇順です。成分の順は縮約 DAG のトポロジカル順で、異なる成分を結ぶ辺は必ず前から後へ向かいます。独立した成分間の順序は保証しません。

`scc_ids()` は成分数と長さ `n` の成分 ID 配列を返します。同じ成分の頂点は同じ ID を持ち、ID は `scc()` の成分番号と一致します。結果を保持しないため、呼ぶたびに再計算します。辺追加後も再利用でき、コピーは独立します。

`size()` は頂点数、`edge_count()` は辺数、`reserve(m)` は辺の格納容量を予約します。頂点数は非負の `int`、辺数は `INT_MAX` 以下、辺の端点は `[0,n)` が必要です。

再帰を使わず、単調な頂点番号の辺、入次数 0 の頂点、functional graph の残りの閉路を処理してから、必要な部分だけ反復 Tarjan 法を適用します。共通部品は [scc_core](scc_core.md) にあります。

頂点数を $$n$$、辺数を $$m$$ として、構築は $$O(1)$$、辺追加は償却 $$O(1)$$、分解は $$O(n+m)$$ 時間・$$O(n+m)$$ 追加空間、グラフの保持は $$O(n+m)$$ 空間です。
