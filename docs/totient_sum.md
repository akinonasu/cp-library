---
title: Euler φ関数の prefix sum
documentation_of: //math/number_theory/totient_sum.hpp
---

```cpp
#include "lib/math/number_theory/totient_sum.hpp"
#include "lib/math/mint.hpp"

auto a = totient_sum(10); // 32
auto b = totient_sum<mint>(10000000000ULL);
auto c = totient_sum<__uint128_t>(10000000000ULL);
```

$$\displaystyle\sum_{i=1}^{N}\varphi(i)$$ を計算します。$$\varphi(1)=1$$、$$N=0$$ の返り値は 0 です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `totient_sum<T = std::uint64_t>(n)` | $$\displaystyle\sum_{i=1}^{n}\varphi(i)$$ を `T` で返す | $$O(n^{2/3}/\log n)$$ 時間、$$O(\sqrt n)$$ 空間 |

`n` は非負の `uint64_t` です。整数で返す場合は答えが `T` に収まる必要があります。大きい答えには `__uint128_t`・`__int128_t` を指定できます。整数の中間集計には 128 bit を使います。

modint を指定すると、その法での値を返します。整数からの構築と加減乗算に対応していれば利用でき、逆元は不要です。合成数の法にも対応します。

Euler φ関数の和だけが必要な場合に使います。他の乗法的関数の和や、素数上の prefix sum も必要な場合は [MultiplicativePrefixSum](https://qandori.github.io/cp-library/math/number_theory/multiplicative_prefix_sum.hpp.html) を使えます。

時間計算量は $$O(n^{2/3}/\log n)$$、追加空間は $$O(\sqrt n)$$ です。
