---
title: "Bostan–Mori"
documentation_of: //fps/bostan_mori.hpp
---

```cpp
#include "lib/fps/bostan_mori.hpp"

fps p{1}, q{1, -1, -1};
auto value = bostan_mori(p, q, 10);
```

分母の定数項が非零の有理式から係数を取り出します。係数型は `fps` と同じ法 998244353 の型です。引数は値渡しなので、不要になった配列は `std::move` で渡せます。

| 操作 | 内容・返り値 |
| --- | --- |
| `bostan_mori(p,q,k)` | $$[x^k]p(x)/q(x)$$。$$q_0\ne0$$、$$k\ge0$$。分子の次数が分母以上でも可 |

$$d=\max(\lvert p\rvert,\lvert q\rvert)$$ として、時間 $$O(d\log d\log k)$$、追加空間 $$O(d)$$ です。
