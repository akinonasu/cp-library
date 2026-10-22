---
title: "通常の畳み込み（軽量実装）"
documentation_of: //convolution/convolution_light.hpp
---

`light_convolution` 名前空間で呼び出します。法 998244353 の計算は軽量 FPS を使い、通常の FPS は読み込みません。高速実装と同時に include できます。

```cpp
#include "lib/convolution/convolution_light.hpp"

std::vector<mint> a{1, 2, 3}, b{4, 5};
auto c = light_convolution::convolution(a, b); // {4,13,22,15}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `light_convolution::convolution(a,b)` | 通常の畳み込み $$\displaystyle c_k=\sum_{i+j=k}a_ib_j$$ を返す。係数型から実装を選ぶ | NTT・CRT・FFT: $$O(L\log L)$$、一般の環: $$O(L^{\log_2 3})$$ |

$$\displaystyle c_k = \sum_{i+j=k} a_i b_j$$ を返します。

配列は定数項から順に多項式の係数を並べます。例は $$(1+2x+3x^2)(4+5x)$$ の係数です。返り値は入力と同じ係数型の `vector` で、両方が非空なら長さは `a.size()+b.size()-1`、どちらかが空なら空配列です。入力は変更しません。`mint` の結果を整数に直すには `c[i].val()` を使います。

| 係数型 | 実装・条件 |
| --- | --- |
| `mint`・mod 998244353 の modint | 軽量 FPS の NTT。出力長 $$\le 2^{23}$$ |
| その他の modint | 32 bit 以下の mod。`mod()`・`val()`・`raw()` が必要。出力長 $$\le 2^{24}$$ |
| 64 bit 以下の整数 | CRT による厳密計算。結果が係数型に収まること。出力長 $$\le 2^{24}$$ |
| 浮動小数点・`std::complex` | FFT による近似計算 |
| その他の環 | Karatsuba。`T(0)`・`+=`・`-=`・`*` が必要 |

[各種畳み込みの参考記事](https://noshi91.hatenablog.com/entry/2020/10/27/175112)。添字・集約・要素同士の演算は各ヘッダーの説明に記載しています。

$$L = \lvert a\rvert + \lvert b\rvert$$ として、NTT・CRT・FFT は時間 $$O(L \log L)$$、その他の環は $$O(L^{\log_2 3})$$、追加空間はいずれも $$O(L)$$ です。
