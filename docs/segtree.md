---
title: セグメント木
documentation_of: //data_structure/segtree.hpp
library_category: data_structure/segment_tree
redirect_from:
- /data_structure/segment_tree/segtree.hpp.html
- /lightweight/data_structure/segment_tree/segtree.hpp.html
---

一点代入と、区間の要素を左から右の順に `op` でまとめた結果の取得を扱います。このページでは、まとめる計算を「集約」と呼びます。

`op` は `op(op(a,b),c) == op(a,op(b,c))` を満たします。これは要素の順番を保てば、まとめる括弧の位置を変えても結果が同じになる条件（結合法則）です。

`e()` は空区間の値で、`op(e(),x) == x` と `op(x,e()) == x` を満たします。この値を単位元と呼びます。 `op(a,b) == op(b,a)` は不要なので、行列積や関数合成のように順番で結果が変わる演算にも使えます。

```cpp
#include "lib/data_structure/segtree.hpp"

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }

Segtree<long long, op, e> tree(std::vector<long long>{1, 2, 3});
tree.set(1, 5);
auto sum = tree.prod(0, 3); // 9
int r = tree.max_right(0, [](long long x) { return x <= 6; }); // 2
```

`Segtree(n)` は全要素を `e()` で初期化します。添字は 0 始まり、`[l,r)` は `l` 以上 `r` 未満の添字を含む区間です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Segtree(n = 0)` / `Segtree(values)` | 長さ／値の配列から構築 | $$O(n)$$ |
| `size()` | 要素数 | $$O(1)$$ |
| `set(p,value)` / `get(p)` | 一点代入／取得 | $$O(\log n)$$ / $$O(1)$$ |
| `prod(l,r)` | 左から右の順で区間を集約。空区間なら `e()` | $$O(\log n)$$ |
| `all_prod()` | 全体の集約。空列なら `e()` | $$O(1)$$ |
| `max_right(l,pred)` | `pred(prod(l,r)) == true` となる最大の `r` | $$O(\log n)$$ |
| `min_left(r,pred)` | `pred(prod(l,r)) == true` となる最小の `l` | $$O(\log n)$$ |

`pred` は区間の計算結果を 1 つ受け取り、条件を満たせば `true`、満たさなければ `false` を返す関数です。同じ引数には同じ結果を返し、`pred(e()) == true` が必要です。`max_right` では右へ、`min_left` では左へ区間を伸ばして、一度 `false` になったら `true` に戻らない条件を使います。

例えば使用例の非負の数列では、和が 6 以下かどうかを判定しています。`[0,2)` の和は 6、次の要素まで含めた `[0,3)` の和は 9 なので、`max_right` は 2 を返します。`max_right(n,pred)` は `n`、`min_left(0,pred)` は 0 です。詳しい境界の定義は [遅延セグメント木の境界探索](lazy_segtree.md#境界を探す) と同じです。

長さは非負の `int`、一点引数は `[0,n)`、区間は `0 <= l <= r <= n` が必要です。空列に対応し、コピーは独立します。更新以外の操作は `const` な木にも使えます。演算結果は指定した型で表現できるようにしてください。

長さを $$n$$ として、構築・空間は $$O(n)$$、一点更新・区間集約・境界検索は $$O(\log n)$$、一点取得・全体集約・要素数の取得は $$O(1)$$ です。演算・コピーは $$O(1)$$ とします。
