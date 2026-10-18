---
title: "多項式補間"
documentation_of: //fps/interpolate.hpp
---

```cpp
#include "lib/fps/interpolate.hpp"

std::vector<mint> xs{0, 1, 2}, ys{1, 6, 17};
auto f = interpolate(xs, ys);
```

法は 998244353 です。`xs` と `ys` の長さは等しく、`xs` の値は法上で相異なる必要があります。入力は変更しません。

| 操作 | 内容・返り値 |
| --- | --- |
| `interpolate(xs,ys)` | $$f(x_i)=y_i$$ を満たす次数 $$n$$ 未満の多項式を `FormalPowerSeries<T>` で返す。空の入力は空配列 |

Linux x86_64 / GCC で AVX2 を利用できる大きな入力では、多点評価と共通の lane 木を使い、降下と補間の上昇をまとめて処理します。

点数を $$n$$ として、時間 $$O(n\log^2 n)$$、追加空間 $$O(n\log n)$$ です。
