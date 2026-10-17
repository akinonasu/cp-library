---
title: "LCA"
documentation_of: //graph/lca.hpp
---

```cpp
#include "lib/graph/lca.hpp"

LCA tree(3);
tree.add_edge(0, 1);
tree.add_edge(1, 2);
tree.build();
int v = tree.lca(0, 2);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `LCA(n = 0)` | 頂点数を指定 |
| `add_edge(u,v)` | 無向辺を追加し、辺 ID を返す |
| `build(root = 0)` | 指定した根で前処理 |
| `lca(u,v)` | 最小共通祖先 |
| `dist(u,v)` | 経路の辺数 |
| `is_ancestor(u,v)` | `u` が `v` の祖先か。自身も含む |
| `subtree(v)` | DFS の行きがけ順での部分木の半開区間 `[l,r)`。`l` は `v` の位置 |
| `parent[v]` / `depth[v]` | 親（根は -1）／根からの辺数 |
| `n` / `edges[id]` | 全頂点数／辺の両端 |

無向木を扱い、頂点番号は 0 始まりです。`build(root = 0)` で根を指定します。

前処理・空間計算量は $$O(n \log n)$$、各問い合わせは $$O(1)$$ です。
