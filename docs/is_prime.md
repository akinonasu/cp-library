---
title: "素数判定"
documentation_of: //math/is_prime.hpp
---

```cpp
#include "lib/math/is_prime.hpp"

bool a = is_prime(97); // true
bool b = is_prime(1); // false
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `is_prime(n)` | 64 bit 符号なし整数 `n` が素数かを返す | $$O(\log n)$$ |

非負整数を 1 個ずつ判定するときに使います。`n` は `std::uint64_t` で、64 bit 全域を決定的に判定します。0 と 1 は false、2 は true です。ある上限までの素数を全て列挙する場合は [素数篩](https://qandori.github.io/cp-library/math/prime_sieve.hpp.html) を使えます。

時間は $$O(\log n)$$、空間は $$O(1)$$ です。
