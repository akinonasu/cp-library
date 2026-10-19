# ライブラリの分類

| フォルダ | 内容 |
| --- | --- |
| `tree/` | LCA、HLD、補助木、重心分解、全方位木 DP、Static Top Tree、Link Cut Tree、Permutation Tree |
| `graph/` | 最短経路 |
| `graph/connectivity/` | Low-Link、二辺連結成分、動的連結性 |
| `graph/flow/` | 最大流、最小費用流 |
| `graph/matching/` | 一般マッチング、重み付きマッチング、割当問題 |
| `data_structure/` | Li Chao Tree、区間集合、Treap、Slope Trick、Union-Find |
| `data_structure/segment_tree/` | 動的セグメント木、Segtree Beats、Sortable Segtree |
| `data_structure/wavelet_matrix/` | 静的・動的・重み付き・一点代入 Wavelet Matrix |
| `data_structure/priority_queue/` | Retroactive Priority Queue |
| `data_structure/range_query/` | Sparse Table、Disjoint Sparse Table |
| `math/` | modint、有理数 |
| `math/number_theory/` | 素数、素因数分解、整数根、mod 演算、離散対数、Stern–Brocot Tree |
| `math/combinatorics/` | 階乗、二項係数、二項係数の prefix sum |
| `math/floor_sum/` | Floor Sum、Mod of Linear |
| `math/linear_algebra/` | 行列、XOR 基底 |
| `convolution/` | 畳み込み |
| `fps/` | 形式的冪級数、多項式 |
| `string/` | 文字列、Eertree |
| `components/` | 変換と共有する内部部品 |

include には新しいパスを使います。型名と関数名は変わりません。

```cpp
#include "lib/tree/lca.hpp"
#include "lib/tree/link_cut_tree.hpp"
#include "lib/graph/flow/max_flow.hpp"
```

説明ページの旧 URL は、新しいページへ転送します。過去の測定結果・凍結した提出コードは当時のパスを保持しています。過去の比較実験は、その測定資料に記録された Git revision で再現してください。
