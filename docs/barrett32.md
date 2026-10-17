---
title: "32 bit Barrett 演算"
documentation_of: //components/barrett32.hpp
---

```cpp
#include "lib/components/barrett32.hpp"

Barrett32 m(1000000007);
auto x = m.mul(123, 456);
```

| 関数・メンバ | 内容・返り値 |
| --- | --- |
| `Barrett32(mod)` | $$1\leq\mathrm{mod}<2^{32}$$ の法で初期化 |
| `reduce(x)` | `uint64_t` の `x` の非負の剰余 |
| `mul(a,b)` | `uint32_t` の `a * b` の非負の剰余 |
| `pow(a,e)` | `uint32_t` の底と `uint64_t` の指数から累乗の非負の剰余 |
| `mod` | 法 |
| `reciprocal` | 剰余計算用の定数 |

構築後にメンバを書き換えないでください。法が 1 の場合は全ての演算が 0 を返します。

構築・`reduce`・`mul` は $$O(1)$$ 時間、`pow` は $$O(\log e)$$ 時間です。追加空間は $$O(1)$$ です。
