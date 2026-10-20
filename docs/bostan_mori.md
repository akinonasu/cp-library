---
title: "Bostan–Mori"
documentation_of: //fps/bostan_mori.hpp
---

```cpp
#include "lib/fps/bostan_mori.hpp"

fps p{1}, q{1, -1, -1};
auto value = bostan_mori(p, q, 10); // 89
```

分母の定数項が非零の有理式から係数を取り出します。係数型は `fps` と同じ法 998244353 の型です。引数は値渡しなので、不要になった配列は `std::move` で渡せます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `bostan_mori(p,q,k)` | $$[x^k]p(x)/q(x)$$。$$q_0\ne0$$、$$k\ge0$$。分子の次数が分母以上でも可 | $$O(d\log d\log k)$$ |

`p[i]`・`q[i]` はそれぞれ $$x^i$$ の係数です。例は $$1/(1-x-x^2)=1+x+2x^2+3x^3+\cdots$$ の第 10 係数を求めています。`k=0` は定数項です。戻り値は係数型 `mint` で、通常の整数が必要なら `value.val()` を使います。

内部の NTT 用作業配列は 64 バイト境界に整列して確保します。Linux の大きな作業配列では huge page を要求し、利用できない環境では通常の整列確保へ切り替えます。

Linux x86_64 / GCC の AVX2 経路では、NTT の倍長化の入力コピーを逆変換の最初へ、係数倍を最後へ融合します。元の変換済み配列を保存しながら、隣接する作業領域へ結果を作ります。

$$d=\max(\lvert p\rvert,\lvert q\rvert)$$ として、時間 $$O(d\log d\log k)$$、追加空間 $$O(d)$$ です。
