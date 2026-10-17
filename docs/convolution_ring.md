---
title: "一般の環の畳み込み"
documentation_of: //convolution/convolution_ring.hpp
---

```cpp
#include "lib/convolution/convolution_ring.hpp"

std::vector<long long> a{1, 2, 3}, b{4, 5};
auto c = convolution_ring(a, b);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `convolution_ring(a,b)` | 一般の環で通常の畳み込みを計算 |

$$c_k = \sum_{i+j=k} a_i b_j$$ を Karatsuba 法で計算します。係数型には `T(0)`・`+=`・`-=`・`*` が必要です。整数型では中間値も型の範囲内とします。

$$L = \lvert a\rvert + \lvert b\rvert$$ として、時間 $$O(L^{\log_2 3})$$、追加空間 $$O(L)$$ です。
