---
title: "線形漸化式の第 k 項"
documentation_of: //fps/linear_recurrence.hpp
---

```cpp
#include "lib/fps/linear_recurrence.hpp"

std::vector<mint> initial{0, 1}, coefficients{1, 1};
auto value = linear_recurrence(initial, coefficients, 10); // 55
```

初期値と係数は `vector<T>` で渡します。法は 998244353 です。内部では [Bostan–Mori](https://qandori.github.io/cp-library/fps/bostan_mori.hpp.html) を使います。

係数が未知で先頭の数項が分かっている場合は、[線形漸化式の復元](berlekamp_massey.md)で係数を求められます。

`initial` は `{a[0],a[1],...,a[d-1]}` の順、`coefficients[0]` は一つ前の項に掛ける係数です。例は `a[0]=0, a[1]=1, a[n]=a[n-1]+a[n-2]` の Fibonacci 数列です。`k` は 0 始まりなので、10 を渡すと `a[10]=55` を返します。戻り値も `mint` です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `linear_recurrence(initial,c,k)` | $$a_n=c_0a_{n-1}+\cdots+c_{d-1}a_{n-d}$$ の $$a_k$$。初期値・係数は各 $$d$$ 個、$$k\ge0$$。$$d=0$$ は `0` | $$O(d\log d\log k)$$ |

係数末尾の零は内部で除き、その次数に対応する末尾の初期値を使います。指定済みの初期値と、漸化式を適用し始める位置は維持します。

漸化式の次数を $$d$$ として、時間 $$O(d\log d\log k)$$、追加空間 $$O(d)$$ です。
