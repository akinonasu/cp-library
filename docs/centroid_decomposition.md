---
title: 重心分解
documentation_of: //data_structure/advanced/centroid_decomposition.hpp
redirect_from:
- /graph/centroid_decomposition.hpp.html
- /tree/centroid_decomposition.hpp.html
---

```cpp
#include "lib/data_structure/advanced/centroid_decomposition.hpp"

CentroidDecomposition<> tree(3);
tree.add_edge(0, 1);
tree.add_edge(1, 2);
tree.build();
int root = tree.root;
int distance = tree.dist(0, 2);
tree.for_each_ancestor(0, [&](int c, int d, int branch) {
	// c までの距離が d。branch は重複の除外に使う。
});
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `CentroidDecomposition<T = int>(n = 0)` | 頂点数と距離の型を指定 | $$O(n)$$ |
| `add_edge(u,v,weight = 1)` | 重み付き無向辺を追加し、辺 ID を返す | 償却 $$O(1)$$ |
| `build(distances = true)` | 重心分解を構築。`false` なら距離キャッシュを省く | $$O(n\log n)$$ |
| `has_distances()` | 構築済みの距離キャッシュを持つか | $$O(1)$$ |
| `ancestors(v)` | 重心分解木の根から `v` までの `span<const Ancestor>` | $$O(1)$$ |
| `for_each_ancestor(v,f)` | `v` から根の順に `f(c,d,branch)` を呼ぶ | $$O(\log n)$$ |
| `is_ancestor(c,v)` | 重心分解木で `c` が `v` の祖先か。自身も含む | $$O(1)$$ |
| `subtree(c)` | 重心分解木の部分木に対応する `order` の区間 `[in[c],out[c])` | $$O(1)$$ |
| `centroid_lca(u,v)` | 重心分解木の最小共通祖先 | $$O(\log n)$$ |
| `distance_to_ancestor(v,c)` | 元の木での `v` と重心祖先 `c` の距離 | $$O(1)$$ |
| `dist(u,v)` | 元の木の経路上の辺重みの和 | $$O(\log n)$$ |
| `Ancestor{centroid,branch,distance}` | 重心祖先／その祖先から `v` に向かう重心分解木の子／元の木での距離 | $$O(1)$$ |
| `Edge{u,v,weight}` | 辺の両端と重み | $$O(1)$$ |
| `root` | 重心分解木の根。空の木では `-1` | $$O(1)$$ |
| `parent[v]` / `depth[v]` / `size[v]` | 重心分解木の親（根は `-1`）／深さ／部分木の頂点数 | $$O(1)$$ |
| `in[v]` / `out[v]` / `order[i]` | 重心分解木の部分木の左端／右端／DFS 順の頂点 | $$O(1)$$ |
| `n` / `edges[id]` | 全頂点数／入力した辺 | $$O(1)$$ |

入力は頂点番号が 0 始まりの無向木です。空の木も扱えます。根は自動で決まり、辺重みには依存しません。`size[c]` は `c` を重心として選んだ時点の成分の頂点数でもあります。各走査は再帰を使いません。

`T` は辺重みと経路和を表せる型を指定します。重み付き木では `CentroidDecomposition<long long>` などを使います。負の重みも扱えますが、最近傍探索など三角不等式を使う応用では非負の重みが必要です。

`Ancestor::branch` は `centroid` から対象頂点に向かう重心分解木の子の頂点番号で、自身のレコードでは `-1` です。同じ重心の同じ枝に属する頂点を除くことで、距離ごとの総和などの重複を取り除けます。枝の距離も、その親の重心から測ります。

例えば非負重みの木で、各重心について有効な頂点までの最小距離 `best[c]` を保存すれば、頂点の追加と最近傍問い合わせを次のように書けます。`inf` は距離和より十分大きく、加算がオーバーフローしない値にします。

```cpp
tree.for_each_ancestor(v, [&](int c, int d, int) {
	best[c] = std::min(best[c], d);
});
int answer = inf;
tree.for_each_ancestor(v, [&](int c, int d, int) {
	if (best[c] != inf) answer = std::min(answer, best[c] + d);
});
```

`build(false)` でも重心分解木と部分木の区間は取得できます。`ancestors`・`for_each_ancestor`・`distance_to_ancestor`・`dist` には `build(true)` が必要です。`ancestors` の返す参照は再構築・辺の追加で無効になります。構築後の公開配列は参照用です。元の木の LCA が必要なら [LCA](https://qandori.github.io/cp-library/tree/lca.hpp.html) を使います。

構築は $$O(n\log n)$$ 時間、距離キャッシュを持つ場合は $$O(n\log n)$$ 空間、持たない場合は $$O(n)$$ 空間です。`add_edge` は償却 $$O(1)$$、`for_each_ancestor`・`centroid_lca`・`dist` は $$O(\log n)$$、それ以外の問い合わせは $$O(1)$$ です。
