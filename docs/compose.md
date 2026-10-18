---
title: "形式的冪級数の合成"
documentation_of: //fps/compose.hpp
---

```cpp
#include "lib/fps/compose.hpp"

fps f{1, 2, 3}, g{0, 1, 1};
auto result = compose(f, g, 8);
```

`f`・`g` は変更しません。法は 998244353、`deg` は返す係数の個数です。

| 操作 | 内容・返り値 |
| --- | --- |
| `compose(f,g,deg = -1)` | $$f(g(x))\bmod x^{\mathrm{deg}}$$。$$g_0=0$$。省略時の長さは `f.size()` |

`g` の先頭の零に合わせて、結果に寄与しない `f` の高次係数を除きます。`g` の非零係数の次数に公約数がある場合は、その間隔で圧縮して計算します。

Linux x86_64 / GCC と AVX2 では、零となる列の変換を省く専用の Kinoshita–Li 法を使います。外側入力の Montgomery 表現を維持し、余分な係数変換を省きます。

入出力の最大係数数を $$n$$ として、時間 $$O(n\log^2 n)$$、追加空間 $$O(n\log n)$$ です。
