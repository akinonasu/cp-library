---
title: "AND・OR・XOR 畳み込み"
documentation_of: //convolution/bitwise_convolution.hpp
---

```cpp
#include "lib/convolution/bitwise_convolution.hpp"

std::vector<long long> a{1, 2, 3, 4}, b{5, 6, 7, 8};
auto c = and_convolution(a, b); // {103,52,73,32}
auto d = or_convolution(a, b); // {5,28,43,184}
auto f = xor_convolution(a, b); // {70,68,62,60}
```

| 関数 | 定義 | 計算量 |
| --- | --- | --- |
| `and_convolution(a, b)` | $$\displaystyle c_k = \sum_{i\mathbin{\&}j=k} a_i b_j$$ | $$O(N\log N)$$ |
| `or_convolution(a, b)` | $$\displaystyle c_k = \sum_{i\mathbin{\vert}j=k} a_i b_j$$ | $$O(N\log N)$$ |
| `xor_convolution(a, b)` | $$\displaystyle c_k = \sum_{i\mathbin{\oplus}j=k} a_i b_j$$ | $$O(N\log N)$$ |

添字 `i,j` に AND・OR・XOR を適用し、その添字へ積 `a[i]*b[j]` を加えます。入力を変更せず、新しい配列を返します。短い配列の不足分は 0 で埋めます。どちらかが空なら空配列です。

長い方の長さを 2 の冪に切り上げた $$N$$ 個を返します。係数は環の要素、XOR ではさらに $$N$$ で割れる型を使います。整数型では中間値も型の範囲内とします。

`bitwise_zeta<inverse, superset>(a)` はその場でゼータ／メビウス変換します。既定は `<false, false>`、長さは 2 の冪です。`walsh_hadamard(a)` は非正規化の Hadamard 変換です。

時間 $$O(N \log N)$$、追加空間 $$O(N)$$ です。変換のみの追加空間は $$O(1)$$ です。
