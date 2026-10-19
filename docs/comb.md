---
title: 階乗・逆階乗と二項係数
documentation_of: //math/combinatorics/comb.hpp
redirect_from:
- /math/comb.hpp.html
---

```cpp
#define COMB_MAX 500000
#include "lib/math/combinatorics/comb.hpp"

init_comb();
auto value = comb(10, 3); // 120
auto factorial = fact[5]; // 120
auto one = fact[5] * finv[5]; // 1
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `init_comb()` | `fact`・`finv` を前計算。使用前に呼ぶ | $$O(N)$$ |
| `comb(n,k)` | 二項係数。`k < 0` または `n < k` なら 0 | $$O(1)$$ |
| `fact[n]` / `finv[n]` | 階乗／逆階乗の前計算値 | $$O(1)$$ |

使用前に `init_comb()` を呼びます。`fact[n]` は $$n!$$、`finv[n]` はその逆数です。mod は 998244353 です。

`init_comb()` を一度呼ぶと、上限までの `comb`・`fact`・`finv` を何度でも使えます。返り値は `mint` なので、通常の整数として取り出す場合は `comb(n,k).val()` とします。

FPS と階乗・逆階乗の生成処理を共有します。256 要素以上の AVX2 対応 x86 では SIMD、それ以外ではスカラー処理を使います。呼び出し方と配列の型は変わりません。

`COMB_MAX` は include 前に指定します。省略時は 502022 です。$$n$$ は前計算範囲内とし、$$\texttt{COMB\_MAX} < 998244353$$ が必要です。

$$N = \texttt{COMB\_MAX}$$ として、`init_comb()` は時間・空間ともに $$O(N)$$、`comb(n, k)` は $$O(1)$$ です。
