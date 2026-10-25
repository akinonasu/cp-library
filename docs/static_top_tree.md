---
title: Static Top Tree
documentation_of: //data_structure/static_top_tree.hpp
redirect_from:
- /graph/static_top_tree.hpp.html
- /tree/static_top_tree.hpp.html
- /data_structure/advanced/static_top_tree.hpp.html
- /lightweight/data_structure/advanced/static_top_tree.hpp.html
library_category: data_structure/advanced
---

木の形を固定したまま、頂点・辺の値の変更に合わせて木 DP を更新します。構築済みの [HLD](https://qandori.github.io/cp-library/data_structure/hld.hpp.html) と、DP の型・演算を渡します。

```cpp
#include "lib/data_structure/static_top_tree.hpp"

struct SumDP {
	using Vertex = long long;
	using Point = long long;
	using Path = long long;
	static Point point_identity() { return 0; }
	static Path path_identity() { return 0; }
	static Point rake(Point a, Point b) { return a + b; }
	static Path compress(Path a, Path b) { return a + b; }
	static Point add_edge(Path a) { return a; }
	static Path add_vertex(Point a, Vertex v) { return a + v; }
};

HLD tree(3);
tree.add_edge(0, 1);
tree.add_edge(1, 2);
tree.build(0);
StaticTopTree<SumDP> dp(tree, {1, 2, 3});
dp.set(1, 4);
long long total = dp.dp();                  // 8
long long subtree = dp.subtree_prod(1);     // 7
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `StaticTopTree<DP>()` | 空の木を作る | $$O(1)$$ |
| `StaticTopTree<DP>(tree,values)` | 構築済みの HLD と頂点番号順の `vector<Vertex>` で構築 | $$O(n\log n)$$ |
| `build(tree,values)` | 木と値を置き換えて再構築 | $$O(n\log n)$$ |
| `size()` / `empty()` | 頂点数／空か | $$O(1)$$ |
| `root()` | 構築時の根。空なら `-1` | $$O(1)$$ |
| `parent(v)` | 元の木の親。根では `-1` | $$O(1)$$ |
| `edge_vertex(e)` | HLD に追加した辺 ID `e` の子側の頂点 | $$O(1)$$ |
| `get(v)` | 頂点 `v` の `const Vertex&` | $$O(1)$$ |
| `set(v,value)` | 頂点の値を変更し、必要なクラスタを更新 | $$O(\log n)$$ |
| `all_prod()` | 木全体の `Path`。境界のパスは構築時の根から heavy path の末尾まで | $$O(1)$$ |
| `dp()` | `DP::add_edge(all_prod())` による木全体の `Point` | $$O(1)$$ |
| `subtree_prod(v)` | 構築時の根に対する `v` の部分木の `Path`。境界のパスは `v` から heavy path の末尾まで | $$O(\log n)$$ |
| `prod(v)` | 木全体を、構築時の根から `v` へのパスに沿って集約した `Path` | $$O(\log n)$$ |
| `Vertex` / `Point` / `Path` | `DP` に指定した型の別名 | —（型） |

頂点は 0 始まりです。空の木・1 頂点の木も扱います。`values.size()` は頂点数と一致させます。構築時に必要な情報をコピーするため、元の HLD は構築後に破棄できます。辺の追加・削除には再構築が必要です。`get` の参照は再構築で無効になります。

DP には次の型と static 関数を定義します。値はコピー・代入可能な型にします。

| 型・関数 | 意味 | 計算量 |
| --- | --- | --- |
| `DP::Vertex` | 頂点の情報。必要なら親辺の情報も含める | —（型） |
| `DP::Point` | 同じ親へ接続する枝の集約 | —（型） |
| `DP::Path` | 境界のパスを持つクラスタの集約 | —（型） |
| `DP::point_identity()` | 枝がない場合の `Point` | DP による（定数時間を仮定） |
| `DP::path_identity()` | 空のパスの `Path` | DP による（定数時間を仮定） |
| `DP::rake(a,b)` | 同じ親へ接続する枝を併合 | DP による（定数時間を仮定） |
| `DP::compress(a,b)` | 上側のパス `a` と下側のパス `b` をこの順で連結 | DP による（定数時間を仮定） |
| `DP::add_edge(path)` | パスクラスタを親へ接続する枝の `Point` に変換 | DP による（定数時間を仮定） |
| `DP::add_vertex(point,vertex)` | 枝の集約に頂点を加え、長さ 1 の `Path` に変換 | DP による（定数時間を仮定） |

`rake` は可換モノイド、`compress` はモノイドにします。`add_edge(path_identity()) == point_identity()` が必要です。クラスタの併合順序によらず同じ木 DP を表すように各演算を定義します。逆元は不要で、積が 0 になる DP も扱えます。

辺を更新する場合は、子の `Vertex` に親辺の値を保存し、`edge_vertex(e)` で子を求めて `set` します。根の仮想的な親辺は、DP に応じた単位元にします。`subtree_prod(v)` にも `Vertex` に保存した `v` の親辺の作用が含まれます。

`prod(v)` は経路上の頂点だけを集約する操作ではありません。経路外の枝を `Point` として畳み、**全頂点を含む**根から `v` へのクラスタを返します。`Path` に両方向の情報を持たせれば、指定した頂点を根とする全方位の値も取り出せます。逆方向の辺・頂点の作用も DP 側で定義します。[一次関数合成の全方位 DP](https://github.com/qandori/cp-library/blob/main/test/fixtures/static_top_tree_dp.hpp) と [可変根の使用例](https://github.com/qandori/cp-library/blob/main/test/point_set_tree_path_composite_sum.test.cpp) を参照してください。独立集合のように枝の方向を反転させる処理が必要な DP では、固定根の `Path` をそのまま逆順にするだけでは全方位の答えになりません。

`all_prod`・`subtree_prod` も `Path` を返すため、固定根の答えとして使う際には DP に応じた取り出し方を使います。例えば行列を `Path` にする DP では、末尾に空の子の状態を作用させます。`dp` はその処理を `add_edge` で行う場合に使えます。

DP の型・演算を定数時間・定数空間とすると、HLD 構築後の構築・再構築は $$O(n\log n)$$ 時間、空間は $$O(n)$$ です。`set`・`subtree_prod`・`prod` は $$O(\log n)$$、それ以外の操作は $$O(1)$$ です。
