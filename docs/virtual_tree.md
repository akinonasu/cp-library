---
title: "Virtual Tree"
documentation_of: //graph/virtual_tree.hpp
---

```cpp
#include "lib/graph/lca.hpp"
#include "lib/graph/virtual_tree.hpp"

LCA tree(5);
tree.add_edge(0, 1);
tree.add_edge(1, 2);
tree.add_edge(1, 3);
tree.add_edge(3, 4);
tree.build();
VirtualTree vt(tree);
std::vector<int> keys{2, 4};
vt.build(keys);
long long length = 0;
for (const auto &node : vt.nodes()) length += node.length;
// length = 3
```

指定頂点と、それらの LCA だけを残した補助木を作ります。[LCA](https://qandori.github.io/cp-library/graph/lca.hpp.html) または [HLD](https://qandori.github.io/cp-library/graph/hld.hpp.html) を先に `build` してください。その根に基づく LCA を使い、補助木の根は指定頂点全体の LCA になります。

| 操作 | 内容・返り値 |
| --- | --- |
| `VirtualTree(tree)` | 前処理済みの木を参照して作成。型は推論可能 |
| `build(vertices)` | 頂点列から構築し、`nodes()` と同じ配列への const 参照を返す |
| `build_sorted(vertices)` | 元の木の `subtree(v).first` の昇順で渡してソートを省略 |
| `size()` | 補助木の頂点数 |
| `empty()` | 補助木が空か |
| `root()` | 根の圧縮 ID。空なら -1、そうでなければ `size() - 1` |
| `operator[](i)` | 圧縮 ID `i` の `Node` への const 参照 |
| `nodes()` | 全 `Node` の配列への const 参照 |

| `Node` の要素 | 内容 |
| --- | --- |
| `vertex` | 元の木での頂点番号 |
| `parent` | 親の圧縮 ID。根は -1 |
| `length` | 親までの元の辺数。根は 0 |
| `selected` | 入力に含まれる頂点か。LCA として追加しただけの頂点は false |

入力は `std::vector<int>`・`std::array<int,k>` などの `std::span<const int>` に変換できる列です。入力を書き換えず、重複は一つにまとめます。空入力にも対応し、異なる指定頂点が $$k$$ 個なら補助木は最大 $$2k-1$$ 頂点です。

圧縮 ID は子から親の順です。根以外は必ず `i < vt[i].parent` なので、ID の昇順で木 DP、降順で根からの伝播ができます。元の頂点番号や DFS の行きがけ順とは異なります。隣接リストが必要な場合は `parent` から作成してください。

重み付き距離は、元の木の根からの重み和を `distance[v]` として `distance[vt[i].vertex] - distance[vt[vt[i].parent].vertex]` で得られます。`length` は重みにかかわらず辺数です。

元の木は補助木より長く生存させ、移動・破棄しないでください。元の木の前処理をやり直した後は補助木も再構築してください。返された配列・要素への参照は、次の構築で内容が変わり、再確保で無効になります。作業配列の容量を再利用し、各構築で元の木の全頂点を走査しません。

`Tree` に必要な操作は `lca(u,v)`、部分木の DFS 区間を返す `subtree(v)`、辺数による深さ `depth[v]` です。

元の頂点数を $$n$$、入力列長を $$k$$ とすると、LCA の前処理は時間・空間 $$O(n \log n)$$、補助木の構築は $$O(k \log k)$$、ソート済みなら $$O(k)$$ です。HLD は前処理・空間 $$O(n)$$、構築 $$O(k \log k + k \log n)$$、ソート済みなら $$O(k \log n)$$ です。補助木の追加空間は最大の入力列長に対し $$O(k)$$、生成後の参照操作は $$O(1)$$ です。
