---
title: "Sortable Segtree"
documentation_of: //data_structure/sortable_segtree.hpp
---

```cpp
#include "lib/data_structure/sortable_segtree.hpp"

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
using Tree = SortableSegtree<int, long long, op, e, true>;
Tree tree(std::vector<std::pair<int, long long>>{{3, 10}, {1, 20}, {2, 30}});
tree.sort(0, 3);
auto first = tree.get(0); // {1,20}
auto sum = tree.prod(0, 2); // 50
tree.sort(0, 3, true);
tree.set(1, 4, 100);
tree.set(0, 7LL);
```

キーによる区間ソート・一点更新・区間集約を行います。各要素は `{Key,S}`、位置は 0 始まり、区間は `[l,r)` です。`op` は値 `S` の結合的な演算、`e()` は単位元です。関数合成・行列積・文字列連結のような非可換演算にも対応します。値に順序や逆元は不要で、デフォルト構築できない型も使えます。

`Key` は bool を除く 64 bit 以下の整数型です。符号付き・負数・64 bit の最小値と最大値に対応します。重複キーも使えますが、同じキーの要素順は未規定で、安定ソートではありません。文字列などをキーにする場合は、必要なキーをあらかじめ座標圧縮してください。キーは集約する値とは独立です。

最後のテンプレート引数 `commutative` は、`op` が可換な場合だけ `true` にします。既定の `false` では前向き・逆向きの両方の積を保持します。

| 操作 | 内容・返り値 |
| --- | --- |
| `Tree()` | 空列を作る。`Tree` はテンプレート引数を指定した型 |
| `Tree(a)` | `vector<pair<Key,S>>` から構築 |
| `assign(a)` | 配列から再構築 |
| `size()` / `empty()` | 要素数／空判定 |
| `get(p)` | 現在の位置 `p` の `{キー,値}` を値で返す |
| `key(p)` | 現在の位置 `p` のキーを返す。`S` をコピーしない |
| `set(p,key,value)` | 現在の位置 `p` のキーと値を変更 |
| `set(p,value)` | キーを保って値だけを変更 |
| `sort(l,r,descending = false)` | キーの昇順に並べ替える。`true` なら降順 |
| `prod(l,r)` | 現在の位置順で値を集約。空区間なら `e()` |
| `all_prod()` | 列全体の積。空列なら `e()` |
| `to_vector()` | 現在の順序で全要素を取得 |
| `clear()` | 主要配列の確保容量を残して空列にする |
| `memory_usage()` | 確保済みの動的領域のバイト数 |
| `swap(other)` | 内容を交換 |

`get`・`key`・`prod` は const 操作です。集約の問い合わせではソート済み区間を分割せず、データ構造を書き換えません。ソート後の位置に対して更新するため、元の要素番号を渡す操作ではありません。挿入・要素削除には対応しません。

コピー・ムーブに対応し、移動元は空列になります。構築後に入力配列は破棄できます。長さは $$2^{29}$$ 未満としてください。

要素数を $$n$$、操作数を $$q$$、キーの bit 数に 31 を足したものを $$w$$ とします。初期構築は $$O(n)$$、`get`・`key` は $$O(w+\log_{64}n)$$、一点更新・`prod` は $$O(w+\log n)$$、`all_prod`・`size`・`empty` は $$O(1)$$、`to_vector` は $$O(n)$$ です。ソートを含む全操作の合計は償却 $$O((n+q)(w+\log n))$$ です。ソート 1 回の最悪時間は区間に含まれるブロック数に依存し、$$O(\log n)$$ ではありません。$$n\leq1$$ の操作は $$O(1)$$ です。

保持空間は $$O(n)$$、最大要素数を $$M$$ として容量を含む確保空間は $$O(M)$$ です。`assign` は $$O(M+n)$$、`clear` は $$O(M)$$、`memory_usage` は $$O(\log_{64}n)$$ です。比較・演算・値のコピーを $$O(1)$$ とします。
