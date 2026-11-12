---
title: 遅延セグメント木
documentation_of: //data_structure/lazy_segtree.hpp
library_category: data_structure/segment_tree
redirect_from:
- /data_structure/segment_tree/lazy_segtree.hpp.html
- /lightweight/data_structure/segment_tree/lazy_segtree.hpp.html
---

数列に対して、区間内の全要素を同じ方法で更新し、区間和などを求めるデータ構造です。値のまとめ方と更新方法を、型と関数で指定して使います。

添字は 0 始まりです。`[l,r)` は `a[l], a[l+1], ..., a[r-1]` を含む区間で、`l == r` なら空の区間です。

## 使用例：区間加算と区間和

各区間について和と要素数を持たせます。区間の全要素に `f` を足すと、和は `f * 要素数` だけ増えます。

```cpp
#include "lib/data_structure/lazy_segtree.hpp"
#include <cassert>
#include <vector>

struct S { long long sum, length; };
S op(S a, S b) { return {a.sum + b.sum, a.length + b.length}; }
S e() { return {0, 0}; }
using F = long long;
S mapping(F f, S x) { return {x.sum + f * x.length, x.length}; }
F composition(F f, F g) { return f + g; }
F id() { return 0; }

int main() {
    LazySegtree<S, op, e, F, mapping, composition, id> tree(
        std::vector<S>{ {1, 1}, {2, 1}, {3, 1} });

    tree.apply(0, 2, 5); // 数列は 6, 7, 3
    assert(tree.get(1).sum == 7);
    assert(tree.prod(0, 3).sum == 16);

    int r = tree.max_right(0, [](S x) { return x.sum <= 13; });
    assert(r == 2); // [0,2) の和は 13、[0,3) の和は 16
    int l = tree.min_left(3, [](S x) { return x.sum <= 10; });
    assert(l == 1); // [1,3) の和は 10、[0,3) の和は 16
}
```

この和の条件で境界を探す場合は、各要素が非負である必要があります。負の値を含むと、区間を伸ばしたときに和が増え続けるとは限りません。計算途中も含めて `long long` の範囲に収めてください。

modint の区間アフィン変換と区間和には、専用の [Range Affine Range Sum](range_affine_range_sum.md) も使えます。

## 指定する型と関数

| 型・関数 | 内容 |
| --- | --- |
| `S` | 1 要素の値と、区間の計算結果を表す型 |
| `S op(S a, S b)` | 左側の区間の結果 `a` と右側の区間の結果 `b` を、この順でまとめる |
| `S e()` | 空区間の計算結果。区間和なら 0 に相当する値 |
| `F` | 更新内容を表す型。例では各要素への加算量 |
| `S mapping(F f, S x)` | 区間内の全要素に更新 `f` を適用したときの計算結果を、更新前の結果 `x` から求める |
| `F composition(F f, F g)` | 更新 `g` の後に更新 `f` を適用する、1 つの更新を返す |
| `F id()` | 値を変更しない更新。例では加算量 0 |

次の条件を満たすように定義します。`a, b, c, x` は `S` の値、`f, g` は `F` の更新です。

- `op(op(a,b),c) == op(a,op(b,c))`。要素の順番を保っていれば、まとめる括弧の位置を変えても結果が同じになる条件です。これを結合法則と呼びます。
- `op(e(),x) == x` と `op(x,e()) == x`。空区間とまとめても結果が変わりません。この `e()` を単位元と呼びます。
- `mapping(id(),x) == x`。
- `mapping(composition(f,g),x) == mapping(f,mapping(g,x))`。合成した更新と、`g`・`f` を順番に適用した結果が一致します。
- `mapping(f,op(a,b)) == op(mapping(f,a),mapping(f,b))`。2 つの区間をまとめてから更新しても、個別に更新してからまとめても、結果が一致します。

これらの式の `==` は結果が同じことを表します。`S` や `F` に比較演算子を実装する必要はありません。各型はコピー・代入できるものを使います。`op(a,b) == op(b,a)` は不要なので、行列積や関数合成のように順番で結果が変わる演算も扱えます。

`composition(f,g)` の順序は特に注意してください。例えば、`g` が「2 を加える」、`f` が「3 倍する」なら、合成した更新は「3 倍して 6 を加える」です。

## 構築

```cpp
LazySegtree<S, op, e, F, mapping, composition, id> tree(int n = 0);
LazySegtree<S, op, e, F, mapping, composition, id> tree(const std::vector<S>& a);
```

- 長さを指定した場合は、長さ `n` の数列を全て `e()` で初期化します。引数を省略すると空の数列です。
- 配列を指定した場合は、その各要素を初期値にします。

長さは非負の `int` で表せる範囲にしてください。上の例のように区間の要素数を `S` に含めるときは、各要素の `length` を 1 にした配列で構築します。`tree(n)` では `length` も 0 になるため、同じ初期状態にはなりません。

## 値の取得と更新

以下の `n` は `size()` が返す要素数です。

| 操作 | 内容・返り値 | 引数の範囲 |
| --- | --- | --- |
| `int size() const` | 要素数を返す | — |
| `void set(int p, S x)` | `a[p]` を `x` に置き換える | `0 <= p < n` |
| `S get(int p)` | `a[p]` の現在の値を返す | `0 <= p < n` |
| `S prod(int l, int r)` | `a[l]` から `a[r-1]` までを、左から右の順に `op` でまとめた結果を返す。空区間なら `e()` | `0 <= l <= r <= n` |
| `S all_prod() const` | 数列全体を `op` でまとめた結果を返す。空列なら `e()` | — |
| `void apply(int p, F f)` | `a[p]` を `mapping(f,a[p])` に置き換える | `0 <= p < n` |
| `void apply(int l, int r, F f)` | `[l,r)` の各要素に更新 `f` を適用する。空区間では何もしない | `0 <= l <= r <= n` |

`prod(l,r)` は `const` な木にも使えます。コピーした木への更新は、元の木に影響しません。

## 境界を探す

`max_right`・`min_left` には、**`S` の値を 1 つ受け取り、条件を満たせば `true`、満たさなければ `false` を返す関数 `pred`** を渡します。使用例の `[](S x) { return x.sum <= 13; }` がその関数です。

同じ値で呼び出したときは同じ結果を返すようにし、呼び出し回数や外部の状態の変更で結果を変えないでください。また、`pred(e()) == true` が必要です。

### `max_right(l,pred)`（返り値は `int`）

`l` を固定して、区間の右端 `r` を探します。`0 <= l <= n` が必要です。

次の 2 条件を満たす `r` を返します。

- `r == l`、または `pred(prod(l,r)) == true`。
- `r == n`、または `pred(prod(l,r+1)) == false`。

`r` を増やして区間を伸ばしたとき、判定が一度 `false` になったら、その後も `false` になる関数を使ってください。この条件のもとで、条件を満たす区間 `[l,r)` の **最大の `r`** が返ります。末尾まで条件を満たせば `n`、最初の 1 要素を加えると満たさなくなるなら `l` です。`l == n` の場合は `n` を返します。

### `min_left(r,pred)`（返り値は `int`）

`r` を固定して、区間の左端 `l` を探します。`0 <= r <= n` が必要です。

次の 2 条件を満たす `l` を返します。

- `l == r`、または `pred(prod(l,r)) == true`。
- `l == 0`、または `pred(prod(l-1,r)) == false`。

`l` を減らして区間を伸ばしたとき、判定が一度 `false` になったら、その後も `false` になる関数を使ってください。この条件のもとで、条件を満たす区間 `[l,r)` の **最小の `l`** が返ります。先頭まで条件を満たせば 0、末尾の 1 要素を加えると満たさなくなるなら `r` です。`r == 0` の場合は 0 を返します。

どちらも関数をテンプレート引数にした `max_right<pred>(l)`・`min_left<pred>(r)` の形式でも呼べます。

## 計算量

要素数を $$n$$ として、構築・使用空間は $$O(n)$$、`size()`・`all_prod()` は $$O(1)$$、その他の操作と境界探索は $$O(\log n)$$ です。`op`・`e`・`mapping`・`composition`・`id`・`pred` の 1 回の計算と型のコピーを $$O(1)$$ としています。

参考：[AC Library の Lazy Segtree 日本語ドキュメント](https://atcoder.github.io/ac-library/production/document_ja/lazysegtree.html)
