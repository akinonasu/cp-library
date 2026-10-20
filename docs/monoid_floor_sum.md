---
title: monoid floor sum
documentation_of: //math/floor_sum/monoid_floor_sum.hpp
redirect_from:
- /math/monoid_floor_sum.hpp.html
---

```cpp
#include "lib/math/floor_sum/monoid_floor_sum.hpp"
#include <string>

std::string op(std::string a, std::string b) { return a + b; }
std::string e() { return ""; }

// main 内
auto result = monoid_floor_sum<std::string, op, e>(3, 5, 2, 1, "X", "Y");
// result = "XXYX"
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `monoid_floor_sum<S,op,e>(n,m,a,b,x,y)` | 下記の floor sum に対応する列を `op` で集約して返す | $$O(\log(n+a+b+m))$$ |

`op` は結合則を満たす演算、`e()` は単位元です。$$n,m,a,b$$ は `uint64_t`、$$m > 0$$ とします。

最初の 4 引数は項数、法、係数、定数項、最後の 2 引数は列の要素 `x,y` です。例は文字列の連結で要素の順番を確認しています。実際には行列の積など、順序に意味のある演算にも使えます。

$$\displaystyle h(i) = \left\lfloor\frac{ai+b}{m}\right\rfloor$$ として、次の列を左から `op` で集約します。累乗は同じ要素の繰り返しです。

$$
y^{h(0)},\ x,\ y^{h(1)-h(0)},\ x,\ y^{h(2)-h(1)},\ \ldots,\ x,\ y^{h(n)-h(n-1)}
$$

時間は $$O(\log(n+a+b+m))$$、追加空間は $$O(1)$$ です。`op`・`e` と `S` のコピーは $$O(1)$$ とします。
