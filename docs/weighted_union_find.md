---
title: 重み付き Union-Find
documentation_of: //data_structure/weighted_union_find.hpp
redirect_from:
- /data_structure/union_find/weighted_union_find.hpp.html
- /lightweight/data_structure/union_find/weighted_union_find.hpp.html
library_category: data_structure/union_find
---

既定では、各頂点に対応する値を `potential` と書き、頂点どうしの値の差を管理します。絶対的な値を決めずに、「頂点 `y` の値が頂点 `x` より `w` 大きい」という関係を追加できます。

```cpp
#include "lib/data_structure/weighted_union_find.hpp"
#include <cassert>

int main() {
	WeightedUnionFind<std::int64_t> uf(3);
	assert(uf.merge(0, 1, 5));
	assert(uf.diff(0, 1) == 5);
	assert(uf.size() == 3 && uf.vertex_count() == 3);
	assert(uf.count() == 2 && uf.component_count() == 2);
	assert(!uf.merge(0, 1, 4));
	assert(uf.component_count() == 2);
}
```

演算を指定すると、加減算以外の関係も同じ API で管理できます。`WeightedUnionFind<T,op,e,inv>` の `op(a,b)` は 2 つの値を組み合わせる関数、`e()` は何も変えない値、`inv(a)` は `a` を打ち消す値を返す関数です。既定では、それぞれ加算・0・符号反転を使います。

```cpp
#include "lib/data_structure/weighted_union_find.hpp"
#include <cassert>
#include <cstdint>

std::uint64_t combine(std::uint64_t a, std::uint64_t b) { return a ^ b; }
std::uint64_t identity() { return 0; }
std::uint64_t inverse(std::uint64_t a) { return a; }

int main() {
	WeightedUnionFind<std::uint64_t, combine, identity, inverse> uf(3);
	assert(uf.merge(0, 1, 10));
	assert(uf.merge(1, 2, 12));
	assert(uf.diff(0, 2) == 6); // 10 XOR 12
	assert(!uf.merge(0, 2, 5));
}
```

以下では頂点数を $$n$$ とします。$$\alpha(n)$$ は逆アッカーマン関数で、増加が非常に遅く、実用的な頂点数では 5 以下です。`T` の演算・コピーは定数時間とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `WeightedUnionFind<T>(n)` / `WeightedUnionFind<T,op,e,inv>(n)` | `n` 頂点を独立した成分として構築 | $$O(n)$$ |
| `size()` / `vertex_count()` | 全頂点数を `int` で返す同じ操作 | 時間・追加空間とも $$O(1)$$ |
| `count()` / `component_count()` | 現在の連結成分数を `int` で返す同じ操作 | 時間・追加空間とも $$O(1)$$ |
| `merge(x,y,w)` | `x` から `y` への関係 `w` を追加。矛盾がなければ `bool` の `true`、矛盾時は `false` | 償却 $$O(\alpha(n))$$ |
| `same(x,y)` | 連結しているかを `bool` で返す | 償却 $$O(\alpha(n))$$ |
| `leader(x)` / `size(x)` | 代表頂点／連結成分の頂点数を `int` で返す | 償却 $$O(\alpha(n))$$ |
| `weight(x)` | 代表頂点から `x` への関係を `T` で返す。既定では代表頂点の値を 0 とした差 | 償却 $$O(\alpha(n))$$ |
| `diff(x,y)` | 連結な `x` から `y` への関係を `T` で返す。既定では $$\mathrm{potential}[y]-\mathrm{potential}[x]$$ | 償却 $$O(\alpha(n))$$ |

演算を省略した場合、`merge(x, y, w)` は $$\mathrm{potential}[y]-\mathrm{potential}[x]=w$$ を追加し、矛盾する場合に `false` を返します。`diff(x, y)` は連結な頂点間の $$\mathrm{potential}[y]-\mathrm{potential}[x]$$ を返します。

`same(x, y)` で連結判定、`leader(x)` で代表元、`size(x)` で連結成分の頂点数を取得できます。頂点番号は 0 始まりです。

頂点数は `0 <= n <= INT_MAX`、頂点の引数は `0 <= x,y < n` とします。`n=0` では頂点数・成分数がともに 0 で、頂点を指定する操作は呼べません。演算を省略する場合、`T` はコピー・代入・ゼロによる構築・加減算・符号反転・等値比較に対応し、`T{}` と `T(0)` がともに 0 を表す型を使います。符号付き整数や modint を利用でき、差の計算や更新途中の値も `T` に収まる必要があります。検索時に内部状態を更新するため、問い合わせも非 `const` です。

演算を指定する場合は、`op`・`e`・`inv` の 3 つを全て指定してください。`T` はコピー・代入・等値比較ができ、以下が成り立つ型と演算を使います。

- `op(op(a,b),c) == op(a,op(b,c))`
- `op(e(),a) == a` かつ `op(a,e()) == a`
- `op(inv(a),a) == e()` かつ `op(a,inv(a)) == e()`

`op(a,b)` と `op(b,a)` が異なっていても使えます。例えば、要素の並べ替えを表す置換の合成や、逆行列を持つ行列の積に対応します。`T{}` や整数の 0 で `T` を構築できる必要はなく、初期値には `e()` を使います。演算の返り値は `T` とし、値をコピーしても同じ関係を表すようにしてください。演算・`inv` の計算・比較・コピーを定数時間とした計算量です。

各頂点の値を `p[v]` と書くと、`merge(x,y,w)` は `op(inv(p[x]),p[y]) == w` を追加し、`diff(x,y)` はその左辺を返します。代表頂点を `r` とすると、`weight(x)` は `op(inv(p[r]),p[x])` です。`x` から `y`、続けて `y` から `z` へ進む関係は、`op(diff(x,y),diff(y,z))` の順に組み合わせます。順序を入れ替えないでください。

最初の例では頂点 1 の値が頂点 0 より 5 大きいという関係を登録しています。絶対的な値を指定する必要はありません。`diff` の前に `same` で連結を確認してください。既存の関係と矛盾する `merge` は false を返し、関係を変更しません。

構築・空間は $$O(n)$$、頂点数・成分数の取得は $$O(1)$$ 時間・追加空間です。頂点を指定するその他の操作は償却 $$O(\alpha(n))$$ 時間、追加空間は最悪 $$O(\log n)$$ です。

参考：[suisen WeightedUnionFind](https://github.com/suisen-cp/cp-library-cpp/blob/main/library/datastructure/union_find/weighted_union_find.hpp)、[maspypy Potentialized UnionFind](https://github.com/maspypy/library/blob/main/ds/unionfind/potentialized_unionfind.hpp)
