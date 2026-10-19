---
title: LCA
documentation_of: //tree/lca.hpp
redirect_from:
- /graph/lca.hpp.html
---

根付き木の 2 頂点に対し、両方の祖先で最も深い頂点を求めます。`add_edge` で木の辺を全て追加し、`build` を 1 回呼んでから問い合わせます。

```cpp
#include "lib/tree/lca.hpp"

LCA tree(3);
tree.add_edge(0, 1);
tree.add_edge(1, 2);
tree.build();
int v = tree.lca(0, 2); // 0
int distance = tree.dist(0, 2); // 2
auto [l, r] = tree.subtree(1);
int count = r - l; // 2
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `LCA(n = 0)` | 頂点数を指定 | $$O(n)$$ |
| `add_edge(u,v)` | 無向辺を追加し、辺 ID を返す | 償却 $$O(1)$$ |
| `build(root = 0)` | 指定した根で前処理 | $$O(n\log n)$$ |
| `lca(u,v)` | 最小共通祖先 | $$O(1)$$ |
| `dist(u,v)` | 経路の辺数 | $$O(1)$$ |
| `is_ancestor(u,v)` | `u` が `v` の祖先か。自身も含む | $$O(1)$$ |
| `subtree(v)` | DFS の行きがけ順での部分木の半開区間 `[l,r)`。`l` は `v` の位置 | $$O(1)$$ |
| `parent[v]` / `depth[v]` | 親（根は -1）／根からの辺数 | $$O(1)$$ |
| `n` / `edges[id]` | 全頂点数／辺の両端 | $$O(1)$$ |

無向木を扱い、頂点番号は 0 始まりです。`build(root = 0)` で根を指定します。

`subtree(v)` の区間は、元の頂点番号の区間ではありません。根から DFS して初めて訪れる順に各頂点へ番号を付けたとき、`v` とその子孫が占める区間 `[l,r)` です。`l` が `v` 自身の DFS 番号、`r-l` が `v` を含む部分木の頂点数です。

辺を追加する段階で閉路のない連結な木にし、空でなければ辺数を `n-1` にしてください。辺や根を変えた後は `build` を呼び直します。距離は重みの合計ではなく辺数です。

前処理・空間計算量は $$O(n \log n)$$、各問い合わせは $$O(1)$$ です。
