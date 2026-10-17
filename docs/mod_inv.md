---
title: "定数時間 mod inv"
documentation_of: //math/mod_inv.hpp
---

```cpp
#include "lib/math/mod_inv.hpp"

ModInv inverse(998244353);
auto x = inverse.inv(3);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `ModInv(prime = 998244353)` | 素数を固定し、逆元用の表を前計算 |
| `mod()` | 法を `uint32_t` で返す |
| `inv(x)` | $$x^{-1}\bmod p$$ を `uint32_t` で返す。$$1\le x<p$$ |

法 $$p$$ は $$2\le p<2^{31}$$ の素数です。法は実行時に指定できます。引数は正規化済みの `uint32_t` を渡し、0 の逆元は扱いません。多数の問い合わせで同じオブジェクトを使い回します。既存の `mint` と組み合わせる場合は `mint::raw(inverse.inv(a.val()))` とします。

小さい分子・分母による有理数表現と逆元表を前計算するため、問い合わせ時に Euclid の互除法や繰り返し二乗法を行いません。累乗も必要なら、逆元機能を含む [ModPow](https://qandori.github.io/cp-library/math/mod_pow.hpp.html) を使います。

前計算は $$O(p^{2/3})$$ 時間・空間、`mod`・`inv` は $$O(1)$$ 時間です。
