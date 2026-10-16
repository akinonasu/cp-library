---
title: "Hungarian Algorithm（ハンガリアン法）"
documentation_of: //graph/hungarian.hpp
---

```cpp
#include "lib/graph/hungarian.hpp"

std::vector<std::vector<int>> a = { {4, 1, 3}, {2, 0, 5} };
Hungarian result(a);
auto cost = result.cost;
auto match = result.row_to_col;
Hungarian maximum(a, true);
```

$$n\le m$$ の費用行列について、各行を異なる列へ割り当てます。`cost` は最小費用、`row_to_col[i]` は行 `i` の割当先、`col_to_row[j]` は列 `j` の割当元（未使用は `-1`）です。第 2 引数を `true` にすると最大費用割当を求めます。

`Hungarian<Cost = long long>` は整数を扱います。費用・距離・双対変数・それらの加減算・総費用が `Cost` に収まる必要があります。最大化では費用を符号反転します。広い計算型が必要な場合は `Hungarian<__int128_t>` などを指定します。

各行の要素が連続する配列・`vector<array<T,m>>` なども渡せます。平坦な配列は `Hungarian(n, m, a.data(), stride = -1, maximize = false)` で渡し、`stride` は隣の行までの要素数（省略時 `m`）です。入力は変更しません。

時間は $$O(n^2m)$$、追加空間は $$O(n+m)$$ です。正方の Monge・逆 Monge 行列は $$O(n^2)$$ で処理します。
