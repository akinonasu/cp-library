---
title: セグメント木
documentation_of: //data_structure/segtree.hpp
library_category: data_structure/segment_tree
redirect_from:
- /data_structure/segment_tree/segtree.hpp.html
- /lightweight/data_structure/segment_tree/segtree.hpp.html
---

一点代入と区間集約を扱います。`op` は結合則を満たす演算、`e()` は単位元です。可換性は不要です。

```cpp
#include "lib/data_structure/segtree.hpp"

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }

Segtree<long long, op, e> tree(std::vector<long long>{1, 2, 3});
tree.set(1, 5);
auto sum = tree.prod(0, 3); // 9
int r = tree.max_right(0, [](long long x) { return x <= 6; }); // 2
```

`Segtree(n)` は全要素を `e()` で初期化します。添字は 0 始まり、区間は半開区間です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Segtree(n = 0)` / `Segtree(values)` | 長さ／値の配列から構築 | $$O(n)$$ |
| `size()` | 要素数 | $$O(1)$$ |
| `set(p,value)` / `get(p)` | 一点代入／取得 | $$O(\log n)$$ / $$O(1)$$ |
| `prod(l,r)` | 左から右の順で区間を集約。空区間なら `e()` | $$O(\log n)$$ |
| `all_prod()` | 全体の集約。空列なら `e()` | $$O(1)$$ |
| `max_right(l,pred)` | `[l,r)` の集約が述語を満たす最大の `r` | $$O(\log n)$$ |
| `min_left(r,pred)` | `[l,r)` の集約が述語を満たす最小の `l` | $$O(\log n)$$ |

境界検索は `pred(e()) == true` で、区間を伸ばすと一度だけ `true` から `false` に変わる述語を指定します。

長さは非負の `int`、一点引数は `[0,n)`、区間は `0 <= l <= r <= n` が必要です。空列に対応し、コピーは独立します。更新以外の操作は `const` な木にも使えます。演算結果は指定した型で表現できるようにしてください。

長さを $$n$$ として、構築・空間は $$O(n)$$、一点更新・区間集約・境界検索は $$O(\log n)$$、一点取得・全体集約・要素数の取得は $$O(1)$$ です。演算・コピーは $$O(1)$$ とします。
