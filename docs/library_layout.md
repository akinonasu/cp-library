# ライブラリの配置とサイトの分類

ヘッダーは次の大分類の直下に置きます。細かい分類は GitHub Pages の一覧に表示します。

| フォルダ | 内容 |
| --- | --- |
| `tree/` | 木のアルゴリズム・データ構造 |
| `graph/` | 最短路、連結性、フロー、マッチングなど |
| `data_structure/` | セグメント木、Union-Find、永続データ構造、Wavelet Matrix、Mo's Algorithm など |
| `math/` | modint、整数論、二項係数、Floor Sum、行列など |
| `convolution/` | 畳み込み |
| `fps/` | 形式的冪級数、多項式 |
| `sequence/` | 最長増加部分列（LIS）、転倒数、連長圧縮（RLE） |
| `geometry/` | 点・直線・線分・円・多角形・凸包・最近点対 |
| `string/` | 文字列のアルゴリズム・データ構造 |
| `components/` | 共有する内部部品 |

リポジトリを `lib/` として配置した場合、次のように include します。

```cpp
#include "lib/math/comb.hpp"
#include "lib/graph/max_flow.hpp"
#include "lib/graph/dijkstra.hpp"
#include "lib/data_structure/persistent_segtree.hpp"
#include "lib/data_structure/union_find.hpp"
```

通常版と軽量版は同じフォルダに配置し、実装が異なる場合はファイル名の `_light` で区別します。

サイトでは「最短路」「フロー」「永続データ構造」「整数論」などの細分類を維持します。分類は各ドキュメントの `library_category` で指定し、省略するとヘッダーのフォルダ名を使います。型名と関数名は変わりません。移動前の説明ページの URL は、対応する通常版・軽量版のページへ転送します。
