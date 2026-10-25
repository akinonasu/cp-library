---
title: Block Cut Tree
documentation_of: //graph/block_cut_tree.hpp
library_category: graph/connectivity
redirect_from:
- /graph/connectivity/block_cut_tree.hpp.html
- /lightweight/graph/connectivity/block_cut_tree.hpp.html
---

無向グラフを二重頂点連結成分に分解し、成分と関節点を頂点とする Block Cut Tree を構築します。元のグラフが非連結なら森になります。

```cpp
#include "lib/graph/block_cut_tree.hpp"

BlockCutTree graph(4);
graph.add_edge(0, 1);
graph.add_edge(1, 2);
graph.add_edge(2, 0);
graph.add_edge(2, 3);
graph.build();

assert(graph.block_count() == 2);
assert(graph.is_articulation(2));
assert(graph.node_id[0] == graph.node_id[1]);
const auto &forest = graph.tree; // vector<vector<int>> の無向森
assert(forest.size() == 3);
auto blocks = graph.groups();  // {0,1,2} と {2,3}（順序未指定）
```

頂点番号は 0 始まりです。多重辺・孤立点・非連結グラフ・空グラフに対応します。自己ループは分解に影響しません。橋の両端も 1 成分、孤立点も 1 頂点の成分とします。関節点は複数の成分に属します。

| 操作・メンバー | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `BlockCutTree(n = 0)` | 元の頂点数を指定 | $$O(1)$$ |
| `add_edge(u,v)` | 無向辺を追加し、0 始まりの辺 ID を返す | 償却 $$O(1)$$ |
| `build()` | 成分・関節点・森を構築。辺を追加した後は再実行 | $$O(n+m)$$ |
| `build_components()` | 成分・関節点・頂点の所属を構築。`tree` は空にする | $$O(n+m)$$ |
| `block_count()` / `node_count()` | 成分数／森の頂点数 | $$O(1)$$ |
| `group(c)` | 成分 `c` の元頂点を `span<const int>` で取得 | $$O(1)$$ |
| `groups()` | 全成分の元頂点を `vector<vector<int>>` にコピー | $$O(n)$$ |
| `articulation` / `is_articulation(v)` | 関節点の一覧／元頂点 `v` が関節点か | $$O(1)$$ |
| `node_id[v]` | 元頂点 `v` に対応する森の頂点番号 | $$O(1)$$ |
| `tree[t]` | 森の頂点 `t` の隣接頂点一覧 | $$O(1)$$ |
| `is_block(t)` | 森の頂点 `t` が成分を表すか | $$O(1)$$ |
| `vertex(t)` | 関節点の頂点なら元頂点番号、成分の頂点なら `-1` | $$O(1)$$ |
| `n` / `edges[id]` | 元の頂点数／辺の両端 | $$O(1)$$ |

成分数を $$B$$、関節点数を $$A$$ とすると、森の頂点番号は `[0,B)` が成分、`[B,B+A)` が関節点です。関節点の頂点は、その関節点を含む全ての成分の頂点と結ばれます。`node_id[v]` は、関節点ならその頂点、そうでなければ所属する成分の頂点を指します。

元の頂点 `u,v` の間を調べるときは、森の `node_id[u],node_id[v]` を使います。異なる木に属するなら元のグラフでも非連結です。木上の経路の内側にある関節点は、元のグラフで `u,v` を分離する頂点になります。

結果は `build()` または `build_components()` 後に参照してください。`build_components()` では `tree` は空になりますが、成分・関節点・頂点番号の問い合わせは利用できます。成分 ID と成分内の頂点順は未指定です。`group()` の参照は再構築・代入・破棄で無効になります。公開配列は直接書き換えず、辺の追加には `add_edge()` を使ってください。追加前に `edges.reserve(m)` で領域を予約できます。

頂点数は `INT_MAX/2` 以下、辺数は `INT_MAX/2` 以下とします。頂点数を $$n$$、辺数を $$m$$ として、構築は $$O(n+m)$$ 時間・空間、成分と森の保持は $$O(n)$$ 空間です。
