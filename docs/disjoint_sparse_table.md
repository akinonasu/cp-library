---
title: "Disjoint Sparse Table"
documentation_of: //data_structure/disjoint_sparse_table.hpp
---

```cpp
#include "lib/data_structure/disjoint_sparse_table.hpp"

int op(int a, int b) { return std::min(a, b); }

DisjointSparseTable<int, op> table(std::vector<int>{4, 2, 7});
auto value = table.prod(0, 2); // 2
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DisjointSparseTable()` / `DisjointSparseTable(a)` | 空／配列から構築 | 空: $$O(1)$$、配列: $$O(n\log n)$$ |
| `build(a)` | 配列で再構築 | $$O(n\log n)$$ |
| `prod(l,r)` / `all_prod()` | $$[l,r)$$／全体の集約。空区間には `e` の指定が必要 | $$O(1)$$ |
| `operator[](i)` | 元の配列の要素を取得 | $$O(1)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |

`op` は結合則を満たす演算です。`prod(l, r)` は $$[l,r)$$ の集約、`all_prod()` は全体の集約です。

空区間も使う場合は単位元を返す関数 `e()` を定義し、`DisjointSparseTable<S, op, e>` とします。`build(a)` で再構築できます。

要素数を $$n$$ として、構築・`build` は時間・空間ともに $$O(n \log n)$$、`prod`・`all_prod`・要素取得は $$O(1)$$ です。`op`・`e` と要素のコピーは $$O(1)$$ とします。
