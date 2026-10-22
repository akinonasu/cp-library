---
title: "遅延セグメント木"
documentation_of: //data_structure/segment_tree/lazy_segtree.hpp
---

```cpp
#include "lib/data_structure/segment_tree/lazy_segtree.hpp"

struct S { long long sum, length; };
S op(S a, S b) { return {a.sum + b.sum, a.length + b.length}; }
S e() { return {0, 0}; }
using F = long long;
S mapping(F f, S x) { return {x.sum + f * x.length, x.length}; }
F composition(F f, F g) { return f + g; }
F id() { return 0; }

LazySegtree<S, op, e, F, mapping, composition, id> tree(
    std::vector<S>{ {1, 1}, {2, 1}, {3, 1} });
tree.apply(0, 2, 5);
auto result = tree.prod(0, 3).sum; // 16
int r = tree.max_right(0, [](S x) { return x.sum <= 13; }); // 2
```

集約 `S` は結合則を満たす `op` と単位元 `e()`、作用 `F` は `mapping(f,x)`、合成 `composition(f,g)`、恒等作用 `id()` を与えます。合成は `g` の後に `f` を適用する順です。作用は区間集約と整合する必要があります。`op` の可換性や `F` の等値比較は不要です。上の例では演算結果が `long long` に収まるようにしてください。

`LazySegtree(n)` は全要素を `e()` で初期化します。区間長を持たせる場合は例のようにベクトルで初期化してください。添字は 0 始まり、区間は半開区間です。

| 操作 | 内容 |
| --- | --- |
| `size()` | 要素数 |
| `set(p,x)` / `get(p)` | 一点代入／取得 |
| `prod(l,r)` / `all_prod()` | 区間／全体の集約 |
| `apply(p,f)` / `apply(l,r,f)` | 一点／区間への作用 |
| `max_right(l,pred)` | `[l,r)` の集約が述語を満たす最大の `r` |
| `min_left(r,pred)` | `[l,r)` の集約が述語を満たす最小の `l` |

境界検索は `pred(e()) == true` で、区間を伸ばすと一度だけ true から false に変わる述語を指定します。`max_right<pred>(l)`・`min_left<pred>(r)` の形式も使えます。

空列・空区間に対応し、コピーは独立します。長さは非負の `int`、一点引数は `[0,n)`、区間は `0 <= l <= r <= n` が必要です。`all_prod()`・`size()` 以外は遅延伝搬で内部状態を変更します。

各節点に未伝搬フラグを持ち、保留中の作用がないときの伝搬と恒等作用との合成を省きます。

長さを $$n$$ として、構築・空間は $$O(n)$$、`size()`・`all_prod()` は $$O(1)$$、その他の操作・境界検索は $$O(\log n)$$ です。各型の演算・コピーは $$O(1)$$ とします。
