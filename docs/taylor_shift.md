---
title: "Taylor Shift"
documentation_of: //fps/taylor_shift.hpp
---

```cpp
#include "lib/fps/taylor_shift.hpp"

fps f{1, 2, 3};
auto shifted = taylor_shift(f, 2);
f = taylor_shift(std::move(f), 3);
```

法は 998244353 です。係数列の長さは変わりません。元の `f.shift(c)` と異なり、結果を返す関数です。元の配列を書き換える場合は返り値を代入してください。不要な入力は `std::move` で渡せます。

| 操作 | 内容・返り値 |
| --- | --- |
| `taylor_shift(f,c)` | $$f(x+c)$$ を返す。`f` は値渡し、`c` は係数型へ変換可能な値 |

係数数を $$n$$ として、時間 $$O(n\log n)$$、追加空間 $$O(n)$$ です。
