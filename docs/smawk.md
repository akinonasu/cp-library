---
title: "SMAWK"
documentation_of: //components/smawk.hpp
redirect_from: /math/smawk.hpp.html
---

```cpp
#include "lib/components/smawk.hpp"

auto value = [](int row, int col) { return (row - col) * (row - col); };
auto best = smawk(3, 4, [&](int row, int old_col, int new_col) {
	return value(row, new_col) < value(row, old_col);
}); // {0,1,2}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `smawk(rows,columns,better)` | 行数・列数を指定し、各行の最良の列番号を返す | $$O(n+m)$$ |
| `smawk(rows,column_ids,better)` | 調べる列番号を順に指定する版 | $$O(n+m)$$ |

完全単調行列の各行の最小値を取る列番号を返します。行列は配列として渡さず、比較関数内で必要な値を計算します。返り値 `best[row]` がその行で選ばれた列番号です。比較関数は新しい列がより良いときに `true` を返し、同値なら先の列を残します。

`columns` は列数、または調べる列番号を順に並べた `std::vector<int>` です。後者ではその順序で完全単調とします。行がある場合は少なくとも 1 列必要です。

各行の最良の列が単調でも、完全単調とは限りません。その場合は、同じ比較関数の形で使える [Monotone Minima](https://qandori.github.io/cp-library/components/monotone_minima.hpp.html) を使います。

行数を $$n$$、列数を $$m$$ として時間・空間は $$O(n+m)$$ です。比較は $$O(1)$$ とします。
