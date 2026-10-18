---
title: "形式的冪級数の合成逆元"
documentation_of: //fps/compositional_inverse.hpp
---

```cpp
#include "lib/fps/compositional_inverse.hpp"

fps f{0, 1, 1};
auto inverse = compositional_inverse(f, 5); // {0,1,-1,2,-5}
```

`f` は変更しません。法は 998244353、`deg` は返す係数の個数です。

返る配列は $$g(x)$$ の定数項から順に並ぶ係数です。例は $$f(x)=x+x^2$$ に対する $$g(x)=x-x^2+2x^3-5x^4$$ を 5 係数で返します。コメントの負数は mod 998244353 上の値を表します。入力条件を満たす `f` を渡すと、指定された長さの逆元が得られます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `compositional_inverse(f,deg = -1)` | $$f(g(x))=x\bmod x^{\mathrm{deg}}$$ となる $$g$$。$$f_0=0$$、長さ 2 以上を求める場合は $$f_1\ne0$$。省略時は `f.size()` | $$O(n\log^2 n)$$ |

Linux x86_64 / GCC と AVX2 では、射影から冪まで同じ作業領域で計算します。

入出力の最大係数数を $$n$$ として、時間 $$O(n\log^2 n)$$、追加空間 $$O(n)$$ です。
