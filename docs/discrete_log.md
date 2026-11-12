---
title: 離散対数
documentation_of: //math/discrete_log.hpp
redirect_from:
- /math/number_theory/discrete_log.hpp.html
- /lightweight/math/number_theory/discrete_log.hpp.html
library_category: math/number_theory
---

```cpp
#include "lib/math/discrete_log.hpp"
#include <iostream>

auto x = discrete_log(2, 8, 13);
if (x) std::cout << *x << '\n'; // 3
else std::cout << -1 << '\n';
```

| 関数 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `discrete_log(a,b,mod)` | $$a^x\equiv b\pmod{\mathrm{mod}}$$ を満たす最小の非負整数を `optional<uint32_t>` で返す。解がなければ `nullopt` | 期待 $$\displaystyle O(\sqrt m+\log^2 m)$$ |

`a` と `b` は `uint64_t`、`mod` は `uint32_t` です。$$1\leq\mathrm{mod}<2^{32}$$ に対応し、合成数や偶数の法、底と法が互いに素でない場合も扱えます。`a` と `b` は法以上でも構いません。負数を扱う場合は、呼び出し前に法で割った余りの等しい 0 以上法未満の整数へ直してください。

$$0^0=1$$ とします。法が 1 の場合は常に 0 を返します。

例では $$2^3\equiv8\pmod{13}$$ なので 3 を返します。戻り値は `std::optional` です。`if (x)` で解の有無を確認し、解があれば `*x` を使います。答えが 0 の場合も `if (x)` は真になります。

固定した素数 mod に対する大量のクエリには、前計算型の [ModPow](https://qandori.github.io/cp-library/math/mod_pow.hpp.html) も使えます。

法を $$m$$ として、期待 $$\displaystyle O(\sqrt m+\log^2 m)$$ 時間、追加空間 $$\displaystyle O(\sqrt m)$$ です。
