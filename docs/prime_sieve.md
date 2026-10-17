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

奇数だけを 1 byte ずつ保持し、小さい入力は一括で、大きい入力は区間ごとに篩います。区間幅は約 32 KiB 以上で、$$\sqrt n$$ がそれを上回る場合は広げます。大きい入力では 3・5・7・11 の倍数を周期パターンで除き、残りの素数の次の倍数を区間間で引き継ぎます。

時間は $$O(n\log\log n)$$、補助空間は $$O(\sqrt n)$$、出力を含む空間は $$O(\sqrt n + \pi(n))$$ です。
