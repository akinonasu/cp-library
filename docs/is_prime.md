---
title: "素数判定"
documentation_of: //math/is_prime.hpp
---

```cpp
#include "lib/math/is_prime.hpp"

bool prime = is_prime(n);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `is_prime(n)` | 64 bit 符号なし整数 `n` が素数かを返す |

`n` は `std::uint64_t` です。64 bit 全域を決定的に判定します。

時間は $$O(\log n)$$、空間は $$O(1)$$ です。
