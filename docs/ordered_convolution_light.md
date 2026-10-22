---
title: "i < j 制約付き畳み込み（軽量実装）"
documentation_of: //convolution/ordered_convolution_light.hpp
---

`light_convolution` 名前空間で呼び出します。法 998244353 の計算は軽量 FPS を使い、通常の FPS は読み込みません。高速実装と同時に include できます。

```cpp
#include "lib/convolution/ordered_convolution_light.hpp"

std::vector<mint> a{1, 2, 3}, b{4, 5, 6};
auto c = light_convolution::ordered_convolution(a, b); // {0,5,6,12,0}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `light_convolution::ordered_convolution(a,b)` | $$\displaystyle c_k=\sum_{i+j=k,\ i<j}a_ib_j$$ | NTT・CRT・FFT: $$O(L\log^2 L)$$、一般の環: $$O(L^{\log_2 3})$$ |

`a` 側の添字が `b` 側の添字より小さい組だけを加えます。例では `(i,j)=(0,1),(0,2),(1,2)` の 3 組です。両入力が非空なら出力長は `a.size()+b.size()-1`、どちらかが空なら空配列です。入力は変更しません。

$$\displaystyle c_k = \sum_{\substack{i+j=k \\ i<j}} a_i b_j$$。係数型の条件は [通常の畳み込み](https://qandori.github.io/cp-library/convolution/convolution.hpp.html) と同じです。

$$L = \lvert a\rvert + \lvert b\rvert$$ として、NTT・CRT・FFT では時間 $$O(L \log^{2} L)$$、一般の環では $$O(L^{\log_2 3})$$、追加空間はいずれも $$O(L)$$ です。
