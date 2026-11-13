---
title: Low-Link
documentation_of: //graph/low_link.hpp
redirect_from:
- /graph/connectivity/low_link.hpp.html
- /lightweight/graph/connectivity/low_link.hpp.html
library_category: graph/connectivity
---

無向グラフで、頂点や辺を 1 つ取り除くと連結成分の数が増えるものを求めます。そのような頂点を関節点、辺を橋と呼びます。

```cpp
#include "lib/graph/low_link.hpp"

LowLink graph(3);
int id = graph.add_edge(0, 1);
graph.add_edge(1, 2);
graph.build();
bool bridge = graph.is_bridge(id); // true
auto vertices = graph.articulation; // {1}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `LowLink(n)` | 頂点数を指定 | $$O(1)$$ |
| `add_edge(u,v)` | 無向辺を追加し、辺 ID を返す | 償却 $$O(1)$$ |
| `build()` | 関節点・橋を計算。辺の追加後は再実行 | $$O(n+m)$$ |
| `is_bridge(id)` | 辺 ID が橋か | $$O(1)$$ |
| `articulation` / `bridges` | 関節点の一覧／橋の辺 ID の一覧 | $$O(1)$$ |
| `ord[v]` / `low[v]` | DFS の訪問順／子孫を経由して戻れる最も早い訪問順（下記参照） | $$O(1)$$ |
| `n` / `edges[id]` | 全頂点数／辺の両端 | $$O(1)$$ |

無向グラフを扱います。頂点数は `0 <= n < INT_MAX`、辺数は `INT_MAX / 2` 以下、頂点番号は `[0,n)` です。`add_edge(u, v)` は 0 始まりの辺 ID を返します。非連結グラフ・自己ループ・多重辺にも対応します。空グラフの `build()` で得られる各配列は空です。

`ord[v]` は DFS で初めて訪れた順の番号です。`low[v]` は、`v` から DFS 木の辺を 0 本以上、子の方向へ進み、その地点から DFS 木以外の辺を高々 1 本使って届く頂点の `ord` の最小値です。`v` 自身の `ord` も候補に含みます。

`build()` 後に `articulation` から関節点、`bridges` から橋の辺 ID、`edges[id]` から辺の両端を取得できます。`is_bridge(id)` で橋か判定できます。辺を追加した後は `build()` を再実行します。

$$n$$ 頂点・$$m$$ 辺として、`build()` は時間・空間ともに $$O(n+m)$$、`add_edge` は償却 $$O(1)$$、`is_bridge` は $$O(1)$$ です。
