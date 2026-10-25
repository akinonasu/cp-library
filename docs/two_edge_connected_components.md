---
title: 二辺連結成分分解
documentation_of: //graph/two_edge_connected_components.hpp
redirect_from:
- /graph/connectivity/two_edge_connected_components.hpp.html
- /lightweight/graph/connectivity/two_edge_connected_components.hpp.html
library_category: graph/connectivity
---

```cpp
#include "lib/graph/two_edge_connected_components.hpp"

TwoEdgeConnectedComponents graph(4);
graph.add_edge(0, 1);
graph.add_edge(1, 2);
graph.add_edge(2, 0);
int bridge = graph.add_edge(2, 3);
graph.build();
bool same = graph.same(0, 2);
auto members = graph.group(graph.component[0]);
auto forest = graph.forest();
```

| 関数・メンバ | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `TwoEdgeConnectedComponents(n = 0)` | 頂点数を指定して構築 | $$O(1)$$ |
| `add_edge(u,v)` | 無向辺を追加し、0 始まりの辺 ID を返す | 償却 $$O(1)$$ |
| `build()` | 二辺連結成分と橋を計算。辺を追加した後は再実行 | $$O(n+m)$$ |
| `component_count()` | 成分数。空グラフでは 0 | $$O(1)$$ |
| `component[v]` | 頂点 `v` が属する成分の ID | $$O(1)$$ |
| `same(u,v)` | 2 頂点が同じ成分に属するか | $$O(1)$$ |
| `group(c)` | 成分 `c` の頂点一覧を、コピーせず `span<const int>` で返す | $$O(1)$$ |
| `groups()` | 全成分の頂点一覧を `vector<vector<int>>` にコピーして返す | $$O(n+k)$$ |
| `bridges` | 橋の辺 ID の一覧 | $$O(1)$$ |
| `is_bridge(id)` | 辺 `id` が橋か | $$O(1)$$ |
| `forest()` | 各成分を縮約した橋の森を `vector<vector<Arc>>` で返す | $$O(k+b)$$ |
| `Arc::to` / `Arc::id` | 森の隣接成分 ID／元のグラフの辺 ID | $$O(1)$$ |
| `n` / `edges[id]` | 頂点数／辺の両端 | $$O(1)$$ |

無向グラフの橋を全て除いた連結成分を求めます。頂点番号は 0 始まりです。多重辺・自己ループ・非連結グラフ・孤立点・頂点数 0 に対応します。孤立点も 1 個の成分になります。

`build()` の後に結果を参照してください。成分 ID は 0 から `component_count()-1` までで、DFS の完了順です。成分内の頂点と橋の順序は未指定です。辺を追加して再構築すると、成分 ID が変わることがあります。

`group(c)` の参照先は再構築・代入・破棄で無効になります。`groups()` と `forest()` は独立したコピーです。森の辺は両方向に格納し、平行辺や自己ループは残りません。元のグラフが非連結なら、結果も森になります。

`n`・`component`・`bridges`・構築済みの `edges` は直接書き換えないでください。辺の追加前に `edges.reserve(m)` で領域を確保できます。頂点数は `INT_MAX` 未満、辺数は `INT_MAX/2` 以下としてください。

CSR と反復 DFS を使います。成分が確定するたびに頂点スタックから取り出し、保存する頂点一覧と作業用スタックに同じ配列を使います。関節点も必要な場合は [Low-Link](https://qandori.github.io/cp-library/graph/low_link.hpp.html) を使えます。

頂点数を $$n$$、辺数を $$m$$、成分数を $$k$$、橋の本数を $$b$$ とすると、`build()` は $$O(n+m)$$ 時間・空間です。`add_edge` は償却 $$O(1)$$、`group`・`same`・`component_count`・`is_bridge` は $$O(1)$$ 時間です。`groups` は $$O(n+k)$$、`forest` は $$O(k+b)$$ 時間・追加空間です。
