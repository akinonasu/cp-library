---
title: "Stern–Brocot Tree"
documentation_of: //math/stern_brocot_tree.hpp
---

```cpp
#include "lib/math/stern_brocot_tree.hpp"

using Tree = SternBrocotTree<>;
Tree node(5, 3);
auto path = node.path();
auto ancestor = node.ancestor(2);
auto common = node.lca(Tree(7, 4));
auto [lower, upper] = node.range();
bool ok = node.move_left(2);
```

正の有理数を Stern–Brocot Tree の頂点として扱います。根は `1/1`、左の子は親より小さく、右の子は親より大きい値です。上の `5/3` は根から `R 1, L 1, R 1` と移動した頂点で、深さ 3、深さ 2 の祖先は `3/2` です。

| 関数・型 | 内容・返り値 |
| --- | --- |
| `SternBrocotTree<Int = std::uint64_t>` | 頂点。`Int` は bool を除く 64 bit 以下の標準整数型 |
| `value_type` | `pair<Int,Int>`。分子・分母の順 |
| `count_type` | `Int` に対応する符号なし整数型。深さと移動回数に使う |
| `distance_type` | `Int` が 32 bit 以下なら `uint64_t`、それ以外なら `__uint128_t` |
| `Run` / `path_type` | `Run{direction,count}` / `vector<Run>`。方向は `'L'` または `'R'`。`Run` は等値比較可能 |
| `SternBrocotTree()` | 根 `1/1` を構築 |
| `SternBrocotTree(numerator,denominator)` / `SternBrocotTree(value)` | 正の分子・分母から構築。約分されていなくてもよい |
| `numerator()` / `denominator()` / `value()` | 現在の既約分数の分子・分母・組 |
| `lower_bound()` / `upper_bound()` / `range()` | 部分木を表す開区間の下限・上限・両方の組 |
| `depth()` / `is_root()` | 根からの辺数 / 根であるか |
| `operator==` / `operator!=` | 同じ頂点であるか / 異なる頂点であるか |
| `move_left(k = 1)` / `move_right(k = 1)` | 同じ方向へ k 回移動。結果が `Int` に収まらなければ `false`、元の頂点を保持 |
| `move_up(k = 1)` | 親へ k 回移動。根を越える場合は `false`、元の頂点を保持 |
| `left(k = 1)` / `right(k = 1)` | 元の頂点を変更せず、同じ方向へ k 回移動した頂点を `optional` で返す。表現不能なら `nullopt` |
| `parent()` | 親を `optional` で返す。根なら `nullopt` |
| `contains(value)` | `value` が自身を含む部分木に属するか。約分されていなくてもよい |
| `is_ancestor(other)` | 自身が `other` の祖先であるか。同じ頂点でも `true` |
| `path()` | 根から自身への経路を連長圧縮した `path_type` で返す |
| `Tree::encode(numerator,denominator)` | 根から指定した正の有理数への圧縮経路を返す |
| `Tree::encode(numerator,denominator,emit)` | 各区間で `emit(direction,count)` を呼ぶ。配列を確保しない |
| `Tree::decode(path)` | `span<const Run>` から頂点を `optional` で返す。方向が不正、または結果が表現不能なら `nullopt` |
| `ancestor(depth)` | 指定した深さの祖先を `optional` で返す。自身より深ければ `nullopt` |
| `Tree::ancestor(value,depth)` | 指定した正の有理数の、指定した深さの祖先を `optional` で返す |
| `lca(other)` / `Tree::lca(value1,value2)` | 最低共通祖先の頂点を返す |
| `distance(other)` | 二頂点間の辺数を `distance_type` で返す |

分子・分母は正で `Int` に収まる値とします。構築時には自然に約分され、0 や無限大を頂点として持つことはできません。`range()` の端点のみ、0 を `{0,1}`、無限大を `{1,0}` で表します。区間には自身も含まれ、端点は含まれません。有限の有理数との四則演算には [Fraction](https://qandori.github.io/cp-library/math/fraction.hpp.html) を使えます。

移動回数・深さは `count_type` で指定します。0 回の移動は成功し、深さ 0 の祖先は根です。`decode()` は空の経路を根として扱い、回数 0 や同方向の連続区間も受け付けます。`encode()`・`path()` の返り値には回数 0 を含まず、方向は交互に並びます。

分子・分母は元の `Int` に収まる必要がありますが、中間積には幅の広い整数型を使います。64 bit 型の二頂点間の距離は 64 bit を超える場合があるため、`distance_type` は 128 bit です。`Tree::ancestor()` と `Tree::lca()` は入力の圧縮経路を配列にせず処理し、共通部分だけが必要なときは途中で打ち切ります。

分子・分母の最大値を $$M$$、圧縮経路の区間数を $$s$$ として、根の構築・値取得・範囲取得・深さ・比較・包含判定・左右への移動・子・親の取得は $$O(1)$$ です。有理数からの構築・`encode()`・`path()`・`ancestor()`・`lca()`・`distance()`・一括 `move_up()` は $$O(\log M)$$ 時間、`decode()` は $$O(s)$$ 時間です。経路を配列で返す場合の空間は $$O(\log M)$$、それ以外の追加空間と頂点一つの保持は $$O(1)$$ です。
