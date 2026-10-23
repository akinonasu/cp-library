---
title: "素数 mod 上の乗法畳み込み（軽量実装）"
documentation_of: //convolution/multiplicative_convolution_light.hpp
---

`light_convolution` 名前空間で呼び出します。法 998244353 の計算は軽量 FPS を使い、通常の FPS は読み込みません。高速実装と同時に include できます。

```cpp
#include "lib/convolution/multiplicative_convolution_light.hpp"

std::vector<mint> a{1, 2, 3}, b{4, 5, 6};
auto c = light_convolution::multiplicative_convolution(a, b); // {35,28,27}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `light_convolution::multiplicative_convolution(a,b)` | 同じ素数長 `P` の列から $$\displaystyle c_k=\sum_{ij\bmod P=k}a_ib_j$$ を返す | $$O(M(P)+P)$$＋因数分解・原始根探索 |

添字 `i,j` の積を `P` で割った余りを出力先にします。係数の計算は入力型の加算・乗算です。例は長さ `P=3` なので、`a[2]*b[2]` は `2*2 % 3 = 1` より `c[1]` に加えます。`c` は入力と同じ型の長さ `P` の配列です。

入力は同じ長さの素数 $$P$$ 個とし、$$\displaystyle c_k = \sum_{ij\bmod P=k} a_i b_j$$ を返します。係数型の条件は [通常の畳み込み](https://qandori.github.io/cp-library/convolution/convolution.hpp.html) と同じです。

原始根探索後、通常の長さ $$P$$ の畳み込み時間を $$M(P)$$ として、時間 $$O(M(P)+P)$$、追加空間 $$O(P)$$ です。原始根探索では $$P-1$$ の素因数分解と候補ごとの $$O(\log^{2} P)$$ の判定を行います。

入力長は 32 bit に収まるため、原始根の探索に使う $$P-1$$ の相異なる素因数は試し割りで求めます。因数分解に追加で時間 $$O(\sqrt P)$$、空間 $$O(\log P)$$ を使います。
