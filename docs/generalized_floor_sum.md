---
title: "Generalized Floor Sum（一般化 floor sum）"
documentation_of: //math/generalized_floor_sum.hpp
---

```cpp
#include "lib/math/generalized_floor_sum.hpp"

auto value = generalized_floor_sum<1, 2>(4, 5, 2, 1); // 5
auto sums = generalized_floor_sum_moments<2>(4, 5, 2, 1);
auto weighted = sums[1][1]; // 5
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `generalized_floor_sum<P,Q,T = __int128_t>(n,m,a,b)` | $$\sum_{i=0}^{n-1}i^P\lfloor(ai+b)/m\rfloor^Q$$ を返す | $$O(D^3\log m)$$、初回は別途 $$O(D^3)$$ |
| `generalized_floor_sum_moments<D,T = __int128_t>(n,m,a,b)` | $$p+q\le D$$ の和を `sums[p][q]` にまとめて返す | $$O(D^3\log m)$$、初回は別途 $$O(D^3)$$ |

`generalized_floor_sum<P, Q, T = __int128_t>(n, m, a, b)` は次を返します。

$$\sum_{i=0}^{n-1} i^P\left\lfloor\frac{ai+b}{m}\right\rfloor^Q$$

`generalized_floor_sum_moments<D, T = __int128_t>` は $$p+q\le D$$ の和を `sums[p][q]` にまとめて返します。

引数は `uint64_t`、$$m>0$$ です。`T` は符号付き整数または modint を指定します。整数では係数・中間値を含めて `T` に収まる必要があり、modint の標数は $$D+1$$ より大きい必要があります。

引数は順に項数 `n`、法 `m`、係数 `a`、定数項 `b` です。テンプレート引数 `P,Q` はコンパイル時に指定します。上の例では床関数の値が `{0,0,1,1}` なので、`value` は $$0\cdot0^2+1\cdot0^2+2\cdot1^2+3\cdot1^2=5$$ です。`sums[p][q]` は同じ入力に対する `generalized_floor_sum<p,q>` の値で、`p+q <= D` の範囲を参照します。

$$D=P+Q$$ として、時間は $$O(D^3\log m)$$、追加空間は $$O(D^2\log m)$$ です。次数 1・2 は $$O(\log m)$$、次数 1 の追加空間は $$O(1)$$ です。係数の初回前計算は $$O(D^3)$$ 時間・$$O(D^2)$$ 空間です。
