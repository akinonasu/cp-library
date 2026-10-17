---
title: "素数篩"
documentation_of: //math/prime_sieve.hpp
---

```cpp
#include "lib/math/prime_sieve.hpp"

auto primes = prime_sieve(n);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `prime_sieve(n)` | `n` 未満の素数を昇順の `vector<std::size_t>` で返す |

$$n$$ 未満の素数を昇順の `std::vector<std::size_t>` で返します。

奇数だけを 1 byte ずつ保持して篩います。

時間は $$O(n\log\log n)$$、空間は $$\lfloor n/2\rfloor$$ byte と出力分です。
