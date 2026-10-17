---
title: "Sparse Table"
documentation_of: //data_structure/sparse_table.hpp
---

```cpp
#include "lib/data_structure/sparse_table.hpp"

int op(int a, int b) { return std::min(a, b); }

SparseTable<int, op> table(std::vector<int>{4, 2, 7});
auto value = table.prod(0, 2);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `SparseTable()` / `SparseTable(a)` | 空／配列から構築 |
| `build(a)` / `build(n,value)` | 配列／各要素を `value(i)` とする列で再構築 |
| `prod(l,r)` / `all_prod()` | $$[l,r)$$／全体の集約。空区間には `e` の指定が必要 |
| `prod_unchecked(l,r)` | 境界検査なしで非空区間を集約 |
| `operator[](i)` | 元の配列の要素を取得 |
| `size()` / `empty()` | 要素数／空判定 |
| `clear()` | 空に戻す |

`op` は結合則と冪等性を満たす演算です。`prod(l, r)` は $$[l,r)$$、`all_prod()` は全体の集約です。

空区間も使う場合は単位元を返す関数 `e()` を定義し、`SparseTable<S, op, e>` とします。`build(a)` または `build(n, value)` で再構築でき、後者は `value(i)` を各要素とします。

`prod_unchecked(l, r)` は境界検査を省略します。非空の区間を指定してください。

要素数を $$n$$ として、構築は時間・空間ともに $$O(n\log n)$$、集約・要素取得は $$O(1)$$ です。演算とコピーは $$O(1)$$ とします。
