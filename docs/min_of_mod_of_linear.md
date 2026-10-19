---
title: Min of Mod of Linear
documentation_of: //math/floor_sum/min_of_mod_of_linear.hpp
redirect_from:
- /math/min_of_mod_of_linear.hpp.html
---

```cpp
#include "lib/math/floor_sum/min_of_mod_of_linear.hpp"

auto value = min_of_mod_of_linear(4, 5, 2, 1); // 0
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `min_of_mod_of_linear(n,m,a,b)` | $$\min_{0\le x<n}((ax+b)\bmod m)$$ を返す。`n,m > 0` | $$O(\log m)$$ |

$$\min_{0 \le x < n} ((ax+b)\bmod m)$$ を返します。引数・返り値は `std::uint64_t`、$$n,m > 0$$ です。

引数の順番は「項数 `n`、法 `m`、係数 `a`、定数項 `b`」です。例の余りの列は `{1,3,0,2}` なので、返り値は 0 です。`a,b` は法以上でも渡せます。空の列は扱いません。

時間 $$O(\log m)$$、追加空間 $$O(1)$$ です。
