---
title: "Lazy Reversible Treap（遅延伝搬・反転可能乱択平衡二分木）"
documentation_of: //data_structure/lazy_reversible_treap.hpp
---

```cpp
#include "lib/data_structure/lazy_reversible_treap.hpp"
#include "lib/math/mint.hpp"

struct F { mint a, b; };
mint op(mint a, mint b) { return a + b; }
mint e() { return 0; }
mint mapping(F f, mint x, int n) { return f.a * x + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }

using Tree = LazyReversibleTreap<mint, op, e, F, mapping, composition, id, true>;
Tree tree(std::vector<mint>{1, 2, 3});
tree.insert(1, 10);
tree.apply(0, 3, {2, 1});
tree.reverse(1, 4);
auto sum = tree.prod(0, 3);
```

添字は 0 始まり、区間は $$[l,r)$$ です。引数なしで空列、`Tree(vector<S>, seed = 0)` で初期列を構築します。`Tree(seed)` は空列の乱数シードを指定します。

`op` は結合的な演算、`e()` は単位元です。`mapping(f,x,n)` は長さ $$n$$ の積 $$x$$ に更新 $$f$$ を適用し、各要素への同じ更新と整合する必要があります。`composition(f,g)` は $$g$$ の後に $$f$$ を適用する更新、`id()` は恒等更新です。

最後のテンプレート引数は、`op` が可換な場合のみ `true` にします。既定の `false` では逆順の積も保持し、非可換な演算に対応します。

| 操作 | 内容・返り値 |
| --- | --- |
| `Tree(seed = 0)` / `Tree(a,seed = 0)` | 空／配列から構築。`Tree` はテンプレート引数を指定した型 |
| `insert(p,x)` | 位置 `p` の直前に挿入 |
| `erase(p)` | 要素を削除し、その値を返す |
| `get(p)` / `set(p,x)` | 1 点の取得／代入 |
| `apply(p,f)` / `apply(l,r,f)` | 1 点／区間に更新を適用 |
| `reverse(l,r)` | 区間を反転 |
| `prod(l,r)` / `all_prod()` | 区間／全体の集約 |
| `rotate(l,m,r)` | $$[l,m)$$ と $$[m,r)$$ を入れ替える |
| `move(l,r,p)` | 区間を切り取り、削除後の位置 `p` の直前へ挿入 |
| `to_vector()` | 全要素を取得 |
| `size()` / `empty()` | 要素数／空判定 |
| `assign(a)` / `clear()` | 配列で再構築／容量を残して空にする |
| `reserve(n)` / `memory_usage()` | ノード領域を予約／確保済み領域のバイト数 |
| `swap(other)` | 内容と乱数生成器の状態を交換 |

要素数と予約容量は $$2^{30}$$ 未満です。削除したノードは再利用します。`clear()` は確保容量を残します。

現在の要素数を $$n$$、最大要素数を $$m$$ とすると、構築は $$O(n)$$、`assign` は $$O(m+n)$$、挿入・削除・1 点操作・区間操作は期待 $$O(\log n)$$、`all_prod`・`size`・`empty`・`memory_usage` は $$O(1)$$、`to_vector` は $$O(n)$$、`reserve`・`clear` は $$O(m)$$ です。空間は $$O(m)$$（予約容量が大きい場合はその容量分）です。演算・更新・値のコピーを $$O(1)$$ とします。
