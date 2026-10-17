---
title: "線形漸化式の第 k 項"
documentation_of: //fps/linear_recurrence.hpp
---

```cpp
#include "lib/fps/linear_recurrence.hpp"

std::vector<mint> initial{0, 1}, coefficients{1, 1};
auto value = linear_recurrence(initial, coefficients, 10);
```

初期値と係数は `vector<T>` で渡します。法は 998244353 です。内部では [Bostan–Mori](https://qandori.github.io/cp-library/fps/bostan_mori.hpp.html) を使います。

| 操作 | 内容・返り値 |
| --- | --- |
| `linear_recurrence(initial,c,k)` | $$a_n=c_0a_{n-1}+\cdots+c_{d-1}a_{n-d}$$ の $$a_k$$。初期値・係数は各 $$d$$ 個、$$k\ge0$$。$$d=0$$ は `0` |

漸化式の次数を $$d$$ として、時間 $$O(d\log d\log k)$$、追加空間 $$O(d)$$ です。
