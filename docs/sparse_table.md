---
title: Sparse Table
documentation_of: //data_structure/range_query/sparse_table.hpp
redirect_from:
- /data_structure/sparse_table.hpp.html
---

```cpp
#include "lib/data_structure/range_query/sparse_table.hpp"

int op(int a, int b) { return std::min(a, b); }

SparseTable<int, op> table(std::vector<int>{4, 2, 7});
auto value = table.prod(0, 2); // 2
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SparseTable()` / `SparseTable(a)` | 空／配列から構築 | 空: $$O(1)$$、配列: $$O(n\log n)$$ |
| `build(a)` / `build(n,value)` | 配列／各要素を `value(i)` とする列で再構築 | $$O(n\log n)$$ |
| `prod(l,r)` / `all_prod()` | $$[l,r)$$／全体の集約。空区間には `e` の指定が必要 | $$O(1)$$ |
| `prod_unchecked(l,r)` | 境界検査なしで非空区間を集約 | $$O(1)$$ |
| `operator[](i)` | 元の配列の要素を取得 | $$O(1)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `clear()` | 空に戻す | 自明に破棄できる型: $$O(\log n)$$、その他: $$O(n\log n)$$ |

`op` は結合則と冪等性を満たす演算です。`prod(l, r)` は $$[l,r)$$、`all_prod()` は全体の集約です。

空区間も使う場合は単位元を返す関数 `e()` を定義し、`SparseTable<S, op, e>` とします。`build(a)` または `build(n, value)` で再構築でき、後者は `value(i)` を各要素とします。

冪等性は `op(x,x)==x` という条件です。最小値・最大値・gcd などに使えます。総和のようにこの条件を満たさない演算には [Disjoint Sparse Table](https://qandori.github.io/cp-library/data_structure/range_query/disjoint_sparse_table.hpp.html) を使います。構築後の要素は固定です。別の配列に変える場合は `build` を呼び直します。

`prod_unchecked(l, r)` は境界検査を省略します。非空の区間を指定してください。

再構築では各段の確保済み領域を再利用します。小さい配列へ作り直しても容量を保持することがあります。各段の要素領域を解放したい場合は `clear()` を使います。`op` が `const S&` を受け取れる場合、演算の引数用のコピーを省きます。

要素数を $$n$$ として、構築時間と格納する要素数は $$O(n\log n)$$、集約・要素取得は $$O(1)$$ です。保持する容量は過去の構築サイズにも依存します。演算とコピーは $$O(1)$$ とします。
