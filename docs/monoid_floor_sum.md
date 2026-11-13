---
title: monoid floor sum
documentation_of: //math/monoid_floor_sum.hpp
redirect_from:
- /math/floor_sum/monoid_floor_sum.hpp.html
- /lightweight/math/floor_sum/monoid_floor_sum.hpp.html
library_category: math/floor_sum
---

```cpp
#include "lib/math/monoid_floor_sum.hpp"
#include <string>

std::string op(std::string a, std::string b) { return a + b; }
std::string e() { return ""; }

// main 内
auto result = monoid_floor_sum<std::string, op, e>(3, 5, 2, 1, "X", "Y");
// result = "XXYX"
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `monoid_floor_sum<S,op,e>(n,m,a,b,x,y)` | 下記の列を左から順に `op` でまとめた `S` の値を返す | $$O(\log(n+a+b+m))$$ |

`op` は `op(op(a,b),c) == op(a,op(b,c))`、`e()` は `op(e(),a) == a` と `op(a,e()) == a` を満たす値を返す関数にします。括弧の付け方を変えても結果が同じで、`e()` を前後に加えても値が変わらない、という条件です。この組をモノイドと呼びます。$$n,m,a,b$$ は `uint64_t`、$$m > 0$$ とします。

最初の 4 引数は項数、法、係数、定数項、最後の 2 引数は列の要素 `x,y` です。例は文字列の連結で要素の順番を確認しています。実際には行列の積など、順序に意味のある演算にも使えます。

$$\displaystyle h(i) = \left\lfloor\frac{ai+b}{m}\right\rfloor$$ として、次の列を左から `op` でまとめます。累乗は同じ要素の繰り返しで、0 乗は `e()` です。

$$
y^{h(0)},\ x,\ y^{h(1)-h(0)},\ x,\ y^{h(2)-h(1)},\ \ldots,\ x,\ y^{h(n)-h(n-1)}
$$

`n=0` では $$y^{h(0)}$$ を返します。`b < m` の場合は `e()` です。

時間は $$O(\log(n+a+b+m))$$、追加空間は $$O(1)$$ です。`op`・`e` と `S` のコピーは $$O(1)$$ とします。
