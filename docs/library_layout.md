# ライブラリの分類

| フォルダ | 内容 |
| --- | --- |
| `tree/` | LCA、補助木、全方位木 DP、Link Cut Tree、Permutation Tree、木のハッシュ、Prüfer Code |
| `graph/` | Euler 路、Functional Graph、Kruskal、トポロジカルソート、2-SAT |
| `graph/shortest_path/` | Dijkstra、Bellman–Ford、Warshall–Floyd、差分制約系 |
| `graph/connectivity/` | SCC、Low-Link、二辺連結成分、Block Cut Tree、動的連結性 |
| `graph/flow/` | 最大流、最小費用流、Project Selection |
| `graph/matching/` | 二部マッチング、一般マッチング、重み付きマッチング、割当問題 |
| `data_structure/` | 区間集合、Treap、Slope Trick、Fenwick Tree、Top K Sum |
| `data_structure/persistent/` | 永続配列・Union-Find・スタック・キュー・Deque・多重集合・ヒープ・セグメント木 |
| `data_structure/union_find/` | Union-Find、重み付き Union-Find、Rollback Union Find |
| `data_structure/li_chao_tree/` | Li Chao Tree、Dynamic Li Chao Tree |
| `data_structure/segment_tree/` | 動的・双対・遅延セグメント木、Sortable Segtree |
| `data_structure/wavelet_matrix/` | 静的・動的・重み付き・一点代入 Wavelet Matrix |
| `data_structure/range_query/` | Sparse Table、Disjoint Sparse Table、Mo 法 |
| `data_structure/advanced/` | HLD、重心分解、Static Top Tree、Segtree Beats、Kinetic Tournament、Retroactive Priority Queue |
| `math/` | modint、有理数 |
| `math/number_theory/` | 素数、素因数分解、整数根、mod 演算、離散対数、Stern–Brocot Tree |
| `math/combinatorics/` | 階乗、二項係数、二項係数の prefix sum |
| `math/floor_sum/` | Floor Sum、Mod of Linear |
| `math/linear_algebra/` | 行列、XOR 基底 |
| `convolution/` | 畳み込み |
| `fps/` | 形式的冪級数、多項式 |
| `sequence/` | 最長増加部分列（LIS）、転倒数、連長圧縮（RLE） |
| `geometry/` | 点・直線・線分・円・多角形・凸包・最近点対 |
| `string/` | 列・文字列、LCS、Eertree、Trie、接尾辞木 |
| `components/` | 変換と共有する内部部品 |

公開ヘッダーは用途別に配置し、永続データ構造はセグメント木・Union-Find も含めて `data_structure/persistent/` にまとめます。共有する内部部品は `components/`、使い方を確認しながら利用する応用ライブラリは `data_structure/advanced/` に置きます。

include には新しいパスを使います。型名と関数名は変わりません。

```cpp
#include "lib/tree/lca.hpp"
#include "lib/tree/link_cut_tree.hpp"
#include "lib/graph/flow/max_flow.hpp"
#include "lib/graph/shortest_path/dijkstra.hpp"
#include "lib/data_structure/persistent/persistent_segtree.hpp"
#include "lib/data_structure/union_find/union_find.hpp"
```

説明ページの旧 URL は、新しいページへ転送します。
