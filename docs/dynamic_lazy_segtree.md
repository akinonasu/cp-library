---
title: Dynamic Lazy Segtree
documentation_of: //data_structure/segment_tree/dynamic_lazy_segtree.hpp
---

巨大な整数座標の列に対する区間更新と区間集約を扱います。区間は全て半開区間です。更新で必要になったノードだけを作り、座標を事前に集めずに使えます。取得と境界探索ではノードを作成せず、内部状態も変更しません。

次は、最初は全要素が 0 の列に対する区間加算・区間和です。`S` に和と区間の長さを保持し、`initial(l,r)` で未更新区間の集約値を返します。

```cpp
#include "lib/data_structure/segment_tree/dynamic_lazy_segtree.hpp"

struct S { long long sum, length; };
S op(S a, S b) { return {a.sum + b.sum, a.length + b.length}; }
S e() { return {0, 0}; }
S mapping(long long f, S x) { return {x.sum + f * x.length, x.length}; }
long long composition(long long f, long long g) { return f + g; }
long long id() { return 0; }
S initial(std::int64_t l, std::int64_t r) { return {0, r - l}; }

using Seg = DynamicLazySegtree<S, op, e, long long, mapping, composition, id, initial>;

int main() {
	Seg seg(-1000000000000LL, 1000000000000LL);
	seg.apply(-3, 5, 2);
	seg.set(1, S{9, 1});
	auto sum = seg.prod(-3, 5).sum; // 23
	auto x = seg.get(1).sum;       // 9
	auto r = seg.max_right(-3, [](S x) { return x.sum <= 6; }); // 0
	(void)sum; (void)x; (void)r;
}
```

テンプレート引数は `DynamicLazySegtree<S,op,e,F,mapping,composition,id,initial,Coord>` です。

| 引数 | 内容 |
| --- | --- |
| `S` / `op(a,b)` / `e()` | 区間の集約値、その結合、空区間の単位元。`op` は結合的で、左から右の順に結合する |
| `F` / `mapping(f,x)` | 更新と、その更新を集約値 `x` に作用させた結果 |
| `composition(f,g)` / `id()` | `g` の後に `f` を行う合成と、何もしない更新 |
| `initial(l,r)` | 未更新の区間 `[l,r)` の集約値。省略すると `e()` を使う |
| `Coord` | 座標型。既定は `std::int64_t`。`bool` を除く 64 bit 以下の整数型 |

`mapping(id(),x)=x`、`mapping(composition(f,g),x)=mapping(f,mapping(g,x))`、`mapping(f,op(x,y))=op(mapping(f,x),mapping(f,y))` が必要です。`op` や更新同士が可換である必要はありません。`S`・`F` はコピー可能にしてください。デフォルト構築や等値比較は不要です。

`initial` は、隣接区間について `op(initial(l,m),initial(m,r))=initial(l,r)` を満たすように定義します。全要素が同じ値でなくても、この関数で集約値を返せれば扱えます。`e()` は空区間の値なので、区間和で長さを持つ場合の `initial(l,r)` の代わりにはなりません。上の例で初期値を全て 1 にするなら `return {r-l,r-l};` とします。

一点代入の `S` も、他の区間と結合できる値を渡します。上の区間和では長さを必ず 1 にします。初期列を渡す場合も、各要素は `S{値,1}` です。`build(a)` は座標範囲を `[0,a.size())` に変更します。`clear()` は、構築時の配列ではなく `initial` が表す状態に戻します。

`max_right(l,f)` は、`f(prod(l,r))` が真になる最大の `r` を返します。`min_left(r,f)` は、`f(prod(l,r))` が真になる最小の `l` を返します。`f(e())` が真で、区間を伸ばすと真から偽へ単調に変化する必要があります。区間和を上限と比較する場合は、各要素が非負であることが必要です。

構築範囲の右端は `Coord` で表せる必要があります。例えば `std::int64_t` の最大値そのものを要素の座標に含めたい場合、その次の右端を表せないため、座標型や座標の取り方を変えてください。分割・サイズ取得では符号付きの差のオーバーフローを避けていますが、`initial` や利用者の演算にも十分な範囲の型を使ってください。

構築範囲の長さを $$W$$、配列の長さを $$n$$、作成済みノード数を $$k$$ とします。各演算と `initial` を $$O(1)$$ とした計算量です。空・長さ 1 の範囲では $$O(1)$$、64 bit 座標でも木の深さは最大 64 です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DynamicLazySegtree(n = 0)` / `DynamicLazySegtree(l,r)` | `[0,n)` / `[l,r)` を `initial` で初期化。作成済みノードは 0 個 | $$O(1)$$ |
| `DynamicLazySegtree(a)` / `build(a)` | `vector<S>` から `[0,a.size())` を構築。非空なら $$2n-1$$ ノードを作る | $$O(n)$$、再構築時は旧ノードの破棄も必要 |
| `apply(l,r,f)` / `apply(p,f)` | 区間 / 一点に更新 `f` を適用 | 償却 $$O(\log W)$$ |
| `set(p,x)` | 一点を `x` に代入 | 償却 $$O(\log W)$$ |
| `prod(l,r)` / `get(p)` | 区間集約 / 一点の値を返す。`const` で呼べる | $$O(\log W)$$ |
| `all_prod()` | 全体の集約値。空なら `e()` | $$O(1)$$ |
| `max_right(l,f)` / `min_left(r,f)` | 条件を満たす最大の右端 / 最小の左端。`const` で呼べる | $$O(\log W)$$ |
| `size()` / `empty()` | 範囲の長さを `Coord` の符号なし型で返す / 空かどうか | $$O(1)$$ |
| `left_bound()` / `right_bound()` | 構築範囲の左端 / 右端 | $$O(1)$$ |
| `node_count()` / `memory_usage()` | 作成済みノード数 / 確保済みノード領域のバイト数 | $$O(1)$$ |
| `reserve(count)` | ノード `count` 個分の領域を予約。更新回数ではなくノード数を指定 | 容量拡張時は $$O(k)$$ |
| `clear()` | 容量と座標範囲を残して `initial` の状態に戻す | $$O(k)$$ |
| `swap(other)` | 内容と座標範囲を交換 | $$O(1)$$ |
| コピー / ムーブ | 独立した複製 / 内容の移動。移動元は同じ範囲の初期状態になる | コピー $$O(k)$$、ムーブ構築 $$O(1)$$、ムーブ代入は旧ノードの破棄も必要 |

空区間の更新は何もせず、集約は `e()` です。更新で作るノードは 1 操作につき $$O(\log W)$$ 個、クエリでは 0 個です。空間は $$O(k)$$、予約容量が大きい場合はその容量分です。ノード数は `INT_MAX-1` 以下にしてください。
