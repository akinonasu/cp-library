---
title: "一般の環の畳み込み"
documentation_of: //convolution/convolution_ring.hpp
---

```cpp
#include "lib/convolution/convolution_ring.hpp"

std::vector<long long> a{1, 2, 3}, b{4, 5};
auto c = convolution_ring(a, b); // {4,13,22,15}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `convolution_ring(a,b)` | 一般の環で通常の畳み込みを計算 | $$O(L^{\log_2 3})$$ |

$$\displaystyle c_k = \sum_{i+j=k} a_i b_j$$ を Karatsuba 法で計算します。係数型には `T(0)`・`+=`・`-=`・`*` が必要です。整数型では中間値も型の範囲内とします。

`vector<T>` を 2 個渡すと `vector<T>` で積を返します。多項式は定数項から並べます。両方が非空なら長さは `a.size()+b.size()-1`、どちらかが空なら空配列です。入力は変更しません。NTT に必要な逆元や原始根を持たない型にも使えます。

$$L = \lvert a\rvert + \lvert b\rvert$$ として、時間 $$O(L^{\log_2 3})$$、追加空間 $$O(L)$$ です。
