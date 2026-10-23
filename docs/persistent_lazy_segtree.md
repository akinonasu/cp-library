---
title: 永続遅延セグメント木
documentation_of: //data_structure/persistent/persistent_lazy_segtree.hpp
redirect_from:
  - /data_structure/segment_tree/persistent_lazy_segtree.hpp.html
  - /lightweight/data_structure/segment_tree/persistent_lazy_segtree.hpp.html
---

区間更新・区間積・境界探索を過去版ごとに行います。任意の過去版から分岐でき、別の版の同じ区間をコピーできます。一点更新だけなら [永続セグメント木](persistent_segtree.md) を使えます。

```cpp
#include "lib/data_structure/persistent/persistent_lazy_segtree.hpp"
#include <cassert>

struct F { long long a, b; };
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
long long mapping(F f, long long x, int n) { return f.a * x + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }
using Tree = PersistentLazySegtree<long long, op, e, F, mapping, composition, id>;

int main() {
	Tree seg(std::vector<long long>{1, 2, 3, 4});
	auto v0 = seg.initial_root();
	auto v1 = seg.apply(v0, 1, 4, {2, 1});
	auto v2 = seg.copy_range(v1, v0, 2, 4);
	assert(seg.prod(v0, 0, 4) == 10);
	assert(seg.all_prod(v1) == 22 && seg.all_prod(v2) == 13);
	assert(seg.max_right(v2, 0, [](long long s) { return s <= 9; }) == 3);
}
```

各更新は新しい `root_type` を返し、引数の版は変わりません。`initial_root()` が最初の版です。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去版から分岐できます。同じオブジェクトから得たルートだけを渡してください。版ごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

`op(a,b)` は結合的な演算、`e()` は単位元です。可換性は不要で、区間積は添字順です。`mapping(f,x,length)` は長さ `length` の区間積 `x` への作用です。`composition(f,g)` は **`g` の後に `f` を適用**する作用、`id()` は恒等作用です。作用は区間の結合と整合し、`mapping(id(),x,length) == x` を満たす必要があります。`S`・`F` はコピー・代入可能な型とし、等値比較・デフォルト構築は不要です。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `Tree(n = 0)` | 長さ `n`、各要素 `e()` の疎な初期版 | $$O(1)$$ |
| `Tree(a)` | `vector<S>` から初期版を作る | $$O(n)$$ |
| `set(root,p,x)` | 位置 `p` に `x` を代入した版 | $$O(\log n)$$ |
| `apply(root,l,r,f)` | `[l,r)` に `f` を作用させた版 | $$O(\log n)$$ |
| `apply(root,p,f)` | 位置 `p` に `f` を作用させた版 | $$O(\log n)$$ |
| `get(root,p)` | 位置 `p` の値 | $$O(\log n)$$ |
| `prod(root,l,r)` | `[l,r)` の積。空区間は `e()` | $$O(\log n)$$ |
| `all_prod(root)` | 全体の積 | $$O(1)$$ |
| `copy_range(root,source,l,r)` | `[l,r)` を `source` の同じ区間で置換した版 | $$O(\log n)$$ |
| `max_right(root,l,f)` | `f(prod(root,l,r))` が真となる最大の右端 | $$O(\log n)$$ |
| `min_left(root,r,f)` | `f(prod(root,l,r))` が真となる最小の左端 | $$O(\log n)$$ |
| `size()` / `empty()` | 固定長／長さが 0 か | $$O(1)$$ |

`0 <= n <= INT_MAX`、一点操作は `0 <= p < n`、区間操作は `0 <= l <= r <= n` とします。境界探索では `f(e())` が真で、区間を伸ばすと真から偽に単調に変化する必要があります。空区間・同じ版・全区間のコピーはノードを作りません。取得・区間積・境界探索もノードを作りません。

`reserve(count)` は全版を合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。更新の時間には領域確保の償却計算量を含みます。

構築は配列指定なら $$O(n)$$、長さ指定なら $$O(1)$$ です。各更新・区間コピー・区間積・境界探索は $$O(\log n)$$ 時間、更新・コピーごとの追加領域は $$O(\log n)$$ です。長さ 0・1 の操作は定数時間です。
