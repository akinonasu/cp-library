---
title: "離散対数"
documentation_of: //math/discrete_log.hpp
---

```cpp
#include "lib/math/discrete_log.hpp"
#include <iostream>

auto x = discrete_log(2, 8, 13);
if (x) std::cout << *x << '\n';
```

| 関数 | 内容・返り値 |
| --- | --- |
| `discrete_log(a,b,mod)` | $$a^x\equiv b\pmod{\mathrm{mod}}$$ を満たす最小の非負整数を `optional<uint32_t>` で返す。解がなければ `nullopt` |

`a` と `b` は `uint64_t`、`mod` は `uint32_t` です。$$1\leq\mathrm{mod}<2^{32}$$ に対応し、合成数や偶数の法、底と法が互いに素でない場合も扱えます。`a` と `b` は法以上でも構いません。負数は呼び出し前に非負の剰余へ正規化してください。

$$0^0=1$$ とします。法が 1 の場合は常に 0 を返します。

最大公約数で非周期部分を取り除き、底の位数を求めて Pohlig–Hellman 法を使います。素数位数の部分問題には baby-step giant-step 法を使い、中国剰余定理で指数を復元します。短い周期や小さい指数は先に確認します。固定した素数 mod と原始根に対する大量のクエリには、前計算型の [ModPow](https://qandori.github.io/cp-library/math/mod_pow.hpp.html) も使えます。

法を $$m$$ として、期待 $$O(\sqrt m+\log^2 m)$$ 時間、追加空間 $$O(\sqrt m)$$ です。位数が小さい素因数に分かれる場合は、法全体を探索する BSGS より探索量を減らせます。ハッシュ表には線形探索を使います。
