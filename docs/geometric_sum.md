---
title: 等比数列の和
documentation_of: //math/util/geometric_sum.hpp
---

公比 $$r$$ の等比数列の先頭 $$n$$ 項の和を求めます。除算を使わないので、$$r=1$$ や、$$r-1$$ の逆元が存在しない法にも使えます。

```cpp
#include "lib/math/util/geometric_sum.hpp"
#include "lib/math/mint.hpp"
#include <cassert>

assert(geometric_sum(3LL, 4) == 40);       // 1 + 3 + 9 + 27
assert(geometric_sum(2LL, 3LL, 4) == 80);  // 2 + 6 + 18 + 54
assert(geometric_sum(-2LL, 4) == -5);      // 1 - 2 + 4 - 8
assert(geometric_sum(1LL, 100) == 100);
assert(geometric_sum(0LL, 0) == 0);

const mint s = geometric_sum(mint(3), 1000000000000000000ULL);
assert(s * mint(2) == mint(3).pow(1000000000000000000LL) - mint(1));
```

| 関数 | 内容・返り値 |
| --- | --- |
| `geometric_sum(ratio, n)` | $$\displaystyle\sum_{i=0}^{n-1}\mathrm{ratio}^i$$ を返す。`ratio` の型 `T` が返り値の型 |
| `geometric_sum(first, ratio, n)` | $$\mathrm{first}\cdot\displaystyle\sum_{i=0}^{n-1}\mathrm{ratio}^i$$ を返す。`first` と `ratio` は同じ型 `T` |

`n` は `bool` 以外の組込み整数型、`__int128_t`、`__uint128_t` で指定します。0 項の和は `T(0)`、負の `n` は `std::invalid_argument` を送出します。既定引数はありません。

`T` には `T(0)`・`T(1)` による構築、コピー、加算 `+`、乗算 `*` が必要です。加算・乗算は結合則と分配則を満たし、0 と 1 はそれぞれ零元と単位元とします。逆元や減算・除算は必要ありません。自前の `mint`、ACL の static/dynamic modint、合成数を法とする modint、多倍長整数にも使えます。

`T` が `bool` 以外の組込み整数型または 128 bit 整数型なら、途中の加算・乗算で型の範囲を超えた場合に `std::overflow_error` を送出します。他の型の演算の安全性はその型に従います。型を広げたい場合は `geometric_sum<__int128_t>(ratio, n)` のように指定できます。型の演算が対応していれば `constexpr` でも使えます。

加算・乗算を定数時間として、時間は $$O(\log n)$$、追加空間は $$O(1)$$ です。`n = 0` は $$O(1)$$ 時間です。
