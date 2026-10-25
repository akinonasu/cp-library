---
title: 整数除算の切り下げ・切り上げ
documentation_of: //math/integer_division.hpp
library_category: math/util
redirect_from:
- /math/util/integer_division.hpp.html
- /lightweight/math/util/integer_division.hpp.html
---

整数の商を負の無限大・正の無限大の方向に丸めます。負の被除数・除数にも使えます。

```cpp
#include "lib/math/integer_division.hpp"
#include <cassert>
#include <cstdint>
#include <limits>

assert(floor_div(-7, 3) == -3);
assert(ceil_div(-7, 3) == -2);
assert(floor_div(7, -3) == -3);
assert(ceil_div(-7, -3) == 3);

const std::int64_t n = std::numeric_limits<std::int64_t>::max();
assert(ceil_div(n, 2) == 4611686018427387904LL);
const auto wide = floor_div<__int128_t>(std::numeric_limits<std::int64_t>::min(), -1);
assert(wide == (__int128_t(1) << 63));
```

| 関数 | 内容・返り値 |
| --- | --- |
| `floor_div(a, b)` | $$\left\lfloor\displaystyle\frac{a}{b}\right\rfloor$$ を返す |
| `ceil_div(a, b)` | $$\left\lceil\displaystyle\frac{a}{b}\right\rceil$$ を返す |

`a` の型を `T`、`b` の型を `U` として、返り値は `std::common_type_t<T, U>` です。`bool` 以外の組込み整数型と `__int128_t`・`__uint128_t` に対応し、両引数の符号の有無は揃えてください。`int64_t` と `int` のように幅が異なる組合せにも使えます。既定の型引数はありません。

`b = 0` は `std::invalid_argument`、返り値の型の最小値を `-1` で割る場合は `std::overflow_error` を送出します。それ以外の有効な入力は型の全範囲を扱えます。必要なら先頭の例のように `floor_div<__int128_t>(a, b)` として計算する型を広げられます。

どちらも `constexpr` に対応し、時間・追加空間は $$O(1)$$ です。
