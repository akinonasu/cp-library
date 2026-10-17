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

入出力の最大係数数を $$n$$ として、時間 $$O(n\log^2 n)$$、追加空間 $$O(n\log n)$$ です。
