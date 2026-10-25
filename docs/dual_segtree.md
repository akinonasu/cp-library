---
title: Dual Segment Tree
documentation_of: //data_structure/dual_segtree.hpp
library_category: data_structure/segment_tree
redirect_from:
- /data_structure/segment_tree/dual_segtree.hpp.html
- /lightweight/data_structure/segment_tree/dual_segtree.hpp.html
---

区間に更新を適用し、指定した 1 点の値を取得する双対セグメント木です。区間和などの集約値は持ちません。

```cpp
#include "lib/data_structure/dual_segtree.hpp"

long long mapping(long long f, long long x) { return x + f; }
long long composition(long long f, long long g) { return f + g; }
long long id() { return 0; }
using Tree = DualSegtree<long long, long long, mapping, composition, id, true>;

Tree seg(6, 0);
seg.apply(1, 5, 3);
seg.apply(2, -1);
long long x = seg.get(2); // 2
seg.set(2, 10);
auto a = seg.values();   // 0, 3, 10, 3, 3, 0
```

`apply(l,r,f)` は半開区間 `[l,r)`、`apply(p,f)` は 1 点に更新を適用します。`set(p,x)` は、それ以前の更新を反映したうえで、その点の値を `x` に置き換えます。例では位置 2 に加算した結果を、最後に 10 で上書きしています。添字は 0 始まりです。

| テンプレート引数・関数 | 定義 |
| --- | --- |
| `S` | 各点が持つ値の型 |
| `F` | 更新を表す型 |
| `mapping(f,x)` | 値 `x` に更新 `f` を適用した `S` を返す |
| `composition(f,g)` | `g` の後に `f` を適用する更新を返す |
| `id()` | 何も変更しない更新を返す |
| `Commutative` | 全ての更新の順序を入れ替えられるなら `true` を指定できる。既定は `false` |

更新の合成には結合法則が必要です。`mapping(id(),x) == x`、`mapping(composition(f,g),x) == mapping(f,mapping(g,x))` を満たすように定義します。`S` と `F` はコピー・代入できる型を使います。`F` 同士の等値比較と、`S`・`F` のデフォルト構築は不要です。ただし `Tree(n)` と初期値を省略する場合は `S{}` を使います。

区間加算のように全ての更新が可換なら、例のように最後の引数に `true` を指定すると、区間更新・一点更新の前の遅延伝播を省略します。後述のアフィン変換や、異なる値への区間代入では、既定の `false` を使ってください。`true` でも `set` はそれ以前の更新を消して上書きするため、経路の遅延伝播を行います。

アフィン変換 $$x\gets ax+b$$ の例です。更新順序によって答えが変わるため、`composition` の引数の順序に注意してください。

```cpp
#include "lib/data_structure/dual_segtree.hpp"
#include "lib/math/mint.hpp"

struct Affine { mint a, b; };
mint mapping(Affine f, mint x) { return f.a * x + f.b; }
Affine composition(Affine f, Affine g) {
	return {f.a * g.a, f.a * g.b + f.b};
}
Affine id() { return {1, 0}; }
using Tree = DualSegtree<mint, Affine, mapping, composition, id>;

Tree seg(std::vector<mint>{1, 2, 3});
seg.apply(0, 3, Affine{2, 3}); // 5, 7, 9
seg.apply(1, 3, Affine{3, 4}); // 5, 25, 31
mint x = seg.get(1);           // 25
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DualSegtree()` | 空の木を作成 | $$O(1)$$ |
| `DualSegtree(n,value = S{})` | 長さ `n`、全ての値を `value` にして構築 | $$O(n)$$ |
| `DualSegtree(a)` | `vector<S>` から構築。`std::move(a)` で初期値の配列を移動できる | $$O(n)$$ |
| `build(a)` | `vector<S>` で構築し直す。保留中の更新は破棄する | $$O(n)$$ |
| `apply(l,r,f)` | `[l,r)` の全ての値に更新を適用。空区間は何もしない | $$O(\log n)$$ |
| `apply(p,f)` | 位置 `p` に更新を適用 | 通常 $$O(\log n)$$、可換版 $$O(1)$$ |
| `get(p)` | 位置 `p` の現在の値を返す。`const` で使用可能 | $$O(\log n)$$ |
| `set(p,value)` | 位置 `p` を `value` で上書き | $$O(\log n)$$ |
| `values()` | 全ての現在値を `vector<S>` で返す | $$O(n)$$ |
| `size()` / `empty()` | 長さ／空か | $$O(1)$$ |

長さは `0 <= n <= INT_MAX / 4`、一点操作は `0 <= p < n`、区間操作は `0 <= l <= r <= n` とします。空の木でも `apply(0,0,f)`・`values()`・`size()`・`empty()` を使えます。

`value_type` は `S`、`update_type` は `F` です。`get` は木を変更しません。`values` は内部の更新を葉まで伝播しますが、その後も更新・取得を続けられます。コピーすると値と保留中の更新を複製します。移動元を再利用するときは `build` で構築し直してください。

比較・写像・合成・コピーを $$O(1)$$ とすると、構築・`build`・`values` は $$O(n)$$ 時間、区間更新・一点取得・一点代入は $$O(\log n)$$ 時間です。一点更新は通常 $$O(\log n)$$、可換版では $$O(1)$$、空区間の更新は $$O(1)$$ です。空間は $$O(n)$$ で、値は葉の分だけ、保留中の更新と有無のフラグは内部ノードの分だけ持ちます。
