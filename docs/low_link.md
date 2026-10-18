---
title: "Low-Link"
documentation_of: //graph/low_link.hpp
---

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
| `ord[v]` / `low[v]` | DFS の訪問順／後退辺を使って到達できる最小の訪問順 | $$O(1)$$ |
| `n` / `edges[id]` | 全頂点数／辺の両端 | $$O(1)$$ |

無向グラフを扱います。頂点番号は 0 始まり、`add_edge(u, v)` は辺 ID を返します。

`build()` 後に `articulation` から関節点、`bridges` から橋の辺 ID、`edges[id]` から辺の両端を取得できます。`is_bridge(id)` で橋か判定できます。辺を追加した後は `build()` を再実行します。

$$n$$ 頂点・$$m$$ 辺として、`build()` は時間・空間ともに $$O(n+m)$$、`add_edge` は償却 $$O(1)$$、`is_bridge` は $$O(1)$$ です。
