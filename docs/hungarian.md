---
title: Hungarian Algorithm（ハンガリアン法）
documentation_of: //graph/hungarian.hpp
redirect_from:
- /graph/matching/hungarian.hpp.html
- /lightweight/graph/matching/hungarian.hpp.html
library_category: graph/matching
---

```cpp
#include "lib/graph/hungarian.hpp"

std::vector<std::vector<int>> a = { {4, 1, 3}, {2, 0, 5} };
Hungarian result(a);
auto cost = result.cost;
auto match = result.row_to_col;
Hungarian maximum(a, true);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Hungarian(a,maximize = false)` | 費用行列から最小費用割当を計算。`true` は最大化 | $$O(n^2m)$$ |
| `Hungarian(n,m,a,stride = -1,maximize = false)` | 平坦な配列から計算。`stride` は行間隔、省略時は `m` | $$O(n^2m)$$ |
| `cost` | 割当の総費用 | $$O(1)$$ |
| `row_to_col[i]` / `col_to_row[j]` | 行の割当先／列の割当元。未使用の列は -1 | $$O(1)$$ |

行数を $$n$$、列数を $$m$$ とする $$0\le n\le m$$ の費用行列について、各行を異なる列へ割り当てます。全ての行の長さを $$m$$ にそろえてください。`cost` は最小費用、`row_to_col[i]` は行 `i` の割当先、`col_to_row[j]` は列 `j` の割当元（未使用は `-1`）です。第 2 引数を `true` にすると最大費用割当を求めます。

`Hungarian<Cost = long long>` は整数を扱います。費用・途中の加減算・総費用が全て `Cost` に収まる必要があります。最大化では費用を符号反転します。広い計算型が必要な場合は `Hungarian<__int128_t>` などを指定します。

各行の要素が連続する配列・`vector<array<T,m>>` なども渡せます。平坦な配列は `Hungarian(n, m, a.data(), stride = -1, maximize = false)` で渡し、`stride` は隣の行までの要素数（省略時 `m`）です。明示する `stride` は `m` 以上にし、各行の `m` 要素を読み取れる領域を用意します。入力は変更しません。

空の行列は費用 0、空の割当配列を返します。平坦な配列で `n=0`、`m>0` を指定した場合は、`row_to_col` が空、`col_to_row` が全て `-1` となり、$$O(m)$$ 時間・空間です。

$$n>0$$ の場合の時間は $$O(n^2m)$$、追加空間は $$O(n+m)$$ です。正方行列で、全ての隣り合う 2 行・2 列について `a[i][j] + a[i+1][j+1] <= a[i][j+1] + a[i+1][j]` を満たす場合（Monge 行列）は $$O(n^2)$$ で処理します。不等号が全て逆向きの場合（逆 Monge 行列）も同様です。
