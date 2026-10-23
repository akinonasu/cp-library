---
title: 永続セグメント木
documentation_of: //data_structure/persistent/persistent_segtree.hpp
redirect_from:
  - /data_structure/segment_tree/persistent_segtree.hpp.html
  - /lightweight/data_structure/segment_tree/persistent_segtree.hpp.html
---

配列の過去の状態を残したまま、一点更新・区間積・境界探索を行います。任意の過去版から分岐でき、別の版の区間をコピーすることもできます。遅延伝搬による区間更新は扱いません。

```cpp
#include "lib/data_structure/persistent/persistent_segtree.hpp"
#include <cassert>
#include <vector>

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
using Tree = PersistentSegtree<long long, op, e>;

int main() {
	Tree seg(std::vector<long long>{1, 2, 3, 4});
	seg.reserve(1000);
	auto v0 = seg.initial_root();
	auto v1 = seg.set(v0, 1, 10);
	auto v2 = seg.multiply(v0, 3, 5);
	auto v3 = seg.copy_range(v1, v2, 2, 4);
	assert(seg.prod(v0, 0, 4) == 10);
	assert(seg.prod(v1, 0, 4) == 18);
	assert(seg.get(v2, 3) == 9);
	assert(seg.all_prod(v3) == 23);
	auto r = seg.max_right(v3, 0, [](long long s) { return s <= 14; });
	assert(r == 3);
}
```

`v0` は最初の配列、`v1` は位置 1 を 10 にした版、`v2` は最初の配列の位置 3 に 5 を加えた版です。`v1` と `v2` は互いに独立しています。`v3` は `v1` の区間 `[2,4)` だけを `v2` の値で置き換えた版です。更新を呼んでも元の版は変わらず、返されたルート ID を保存して新しい版を使います。

`op(a,b)` は結合的な演算、`e()` はその単位元です。和以外に最小値・最大値・行列積・関数合成などを扱えます。区間積は添字の小さい順に計算し、可換性は要求しません。`S` はコピー・ムーブ・代入できる型とし、デフォルト構築と等値比較は不要です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `PersistentSegtree(n = 0)` | 長さ `n`、全要素が `e()` の初期版を用意。実ノードはまだ作らない | $$O(1)$$ |
| `PersistentSegtree(a)` | `std::vector<S>` から初期版を構築 | $$O(n)$$ |
| `initial_root()` | コンストラクタで用意した初期版のルート ID | $$O(1)$$ |
| `set(root,p,x)` | 版 `root` の位置 `p` を `x` に代入した新しいルート ID | 償却 $$O(\log n)$$ |
| `multiply(root,p,x)` | 位置 `p` を `op(現在値,x)` にした新しいルート ID | 償却 $$O(\log n)$$ |
| `get(root,p)` | 版 `root` の位置 `p` の値 | $$O(\log n)$$ |
| `prod(root,l,r)` | 版 `root` の半開区間 `[l,r)` の積。空区間は `e()` | $$O(\log n)$$ |
| `all_prod(root)` | 版 `root` の全体の積 | $$O(1)$$ |
| `copy_range(root,source,l,r)` | `root` の `[l,r)` を `source` の同じ区間で置き換えた新しいルート ID | 償却 $$O(\log n)$$ |
| `max_right(root,l,f)` | `f(prod(root,l,r))` が真である最大の右端 `r` | $$O(\log n)$$ |
| `min_left(root,r,f)` | `f(prod(root,l,r))` が真である最小の左端 `l` | $$O(\log n)$$ |
| `size()` / `empty()` | 配列の長さ／長さが 0 か | $$O(1)$$ |
| `reserve(count)` | 空ノードを除くノード `count` 個分の容量を予約。既存のルート ID は変わらない | 再確保時は現在のノード数に比例 |
| `node_count()` | 空ノードを除く、全版で作成したノード数 | $$O(1)$$ |
| `memory_usage()` | 予約済みノード領域のバイト数。内部の空ノード 1 個分も含む | $$O(1)$$ |

ルート ID の型は `root_type`（`std::uint32_t`）、値の型は `value_type`（`S`）です。ルート ID を変数や `std::vector<Tree::root_type>` にコピーするだけなら、時間・追加領域ともに一定です。返される ID は連番の「版番号」ではないので、版番号から ID への対応は利用者が保存してください。

添字は 0 始まりで、`0 <= n <= INT_MAX`、一点操作は `0 <= p < n`、区間操作は `0 <= l <= r <= n` とします。ルート ID はその木の `initial_root`・更新・区間コピーが返したものを使います。別の木の ID を混ぜることはできません。木のオブジェクト自体をコピーするとノード領域全体を複製します。移動元は再代入してから使ってください。

境界探索では `f(e())` が真で、区間を伸ばすと真から偽へ単調に変わる必要があります。例えば非負の配列なら「和が指定値以下」を使えます。負の値がある配列の和では、この条件を満たすとは限りません。

`get`・`prod`・`all_prod`・境界探索は読み取り専用で、ノードを増やしません。区間コピーは部分木を共有し、全区間のコピー、空区間のコピー、同じ版からのコピーはノードを作りません。版ごとのノード解放は行わず、使わなくなった版のノードも木を破棄するまで保持します。

長さが正のとき、$$h=\lceil\log_4 n\rceil$$ とすると、配列からの構築は $$\displaystyle\sum_{i=0}^{h}\lceil n/4^i\rceil$$ ノード、一点更新は $$h+1$$ ノードを作ります。`reserve` の引数は更新回数ではなく、既存分を含むノード数なので、更新回数からこの上限を見積もって予約できます。ノード数の上限は $$2^{32}-2$$ 個です。

演算・値のコピーを $$O(1)$$ とした計算量です。構築は $$O(n)$$、一点更新・区間積・境界探索・区間コピーは $$O(\log n)$$、更新の領域確保を含む時間は償却計算量です。長さ 0・1 の操作は $$O(1)$$ です。構築後に更新・区間コピーを合計 $$u$$ 回行うと、空間は $$O(n+u\log n)$$、長さだけを指定した疎な初期版では $$O(u\log n)$$ です。予約容量が大きい場合はその容量分を使います。
