---
title: "定数時間 mod pow"
documentation_of: //math/mod_pow.hpp
---

```cpp
#include "lib/math/mod_pow.hpp"

ModPow power(17);
auto a = power.pow(3, 5); // 5
auto b = power.inv(7); // 5
auto k = power.log(5); // 5
auto c = power.pow_root(k); // 5
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `ModPow(prime = 998244353,root = 0)` | 素数を固定し、累乗・逆元・離散対数用の表を前計算。`root = 0` なら原始根を自動探索 | 原始根指定時: 期待 $$O(p)$$、自動探索は本文参照 |
| `mod()` | 法を `uint32_t` で返す | $$O(1)$$ |
| `primitive_root()` | 使用している原始根を `uint32_t` で返す | $$O(1)$$ |
| `pow(a,exponent)` | $$a^{\mathrm{exponent}}\bmod p$$。$$0\le a<p$$、指数は非負の `uint64_t`。$$0^0=1$$ | $$O(1)$$ |
| `inv(x)` | $$x^{-1}\bmod p$$。$$1\le x<p$$ | $$O(1)$$ |
| `log(x)` | 使用している原始根 $$g$$ に対する $$g^k=x\pmod p$$ の最小の非負整数 $$k$$。$$1\le x<p$$ | $$O(1)$$ |
| `pow_root(exponent)` | $$g^{\mathrm{exponent}}\bmod p$$。指数は非負の `uint64_t` | $$O(1)$$ |

法 $$p$$ は $$2\le p<2^{31}$$ の素数です。底と指数は問い合わせごとに変更できます。法と原始根は実行時に指定でき、`root` を省略すると最小の原始根を探索します。返り値はすべて `uint32_t` です。底は正規化済みの値を渡します。

同じ法で多数の累乗・逆元を計算するときに、オブジェクトを一度作って使い回します。例の法 17 では原始根は 3、`log(5)` は「3 を何乗すると 5 になるか」を返します。通常の `pow` の底は原始根以外でも指定できます。大きい法では前計算に時間とメモリを使うため、少数回だけの累乗には `mint::pow` なども使えます。

`pow` は原始根に対する離散対数に変換し、指数を $$p-1$$ で縮約してから表を参照します。有理数近似で参照範囲を小さくするため、法の大きさに比例する全対数表は持ちません。逆元だけが必要なら [ModInv](https://qandori.github.io/cp-library/math/mod_inv.hpp.html) を使うと、離散対数の前計算を省けます。`mint` と組み合わせる場合は `mint::raw(power.pow(a.val(),k))` などとします。

離散対数の前計算では小さい素数の対数を利用した乱択を最大 64 回試し、失敗した場合は BSGS に切り替えます。乱数 seed は法と原始根から固定され、結果と処理は再現可能です。問い合わせには乱択もループも使いません。

法や底を毎回変更する場合や、合成数の法を扱う場合は [discrete_log](https://qandori.github.io/cp-library/math/discrete_log.hpp.html) を使えます。

前計算の空間は $$O(p^{2/3})$$、原始根が与えられた場合の時間はハッシュ探索を期待 $$O(1)$$ として期待 $$O(p)$$ です。原始根の自動探索には、$$p-1$$ の試し割りと候補ごとの累乗判定が追加されます。すべての問い合わせは $$O(1)$$ 時間です。
