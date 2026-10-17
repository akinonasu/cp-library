---
title: "階乗・逆階乗と二項係数"
documentation_of: //math/comb.hpp
---

```cpp
#define COMB_MAX 500000
#include "lib/math/comb.hpp"

init_comb();
auto value = comb(10, 3);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `init_comb()` | `fact`・`finv` を前計算。使用前に呼ぶ |
| `comb(n,k)` | 二項係数。`k < 0` または `n < k` なら 0 |
| `fact[n]` / `finv[n]` | 階乗／逆階乗の前計算値 |

使用前に `init_comb()` を呼びます。`fact[n]` は $$n!$$、`finv[n]` はその逆数です。mod は 998244353 です。

`COMB_MAX` は include 前に指定します。省略時は 502022 です。$$n$$ は前計算範囲内とし、$$\texttt{COMB\_MAX} < 998244353$$ が必要です。

$$N = \texttt{COMB\_MAX}$$ として、`init_comb()` は時間・空間ともに $$O(N)$$、`comb(n, k)` は $$O(1)$$ です。
