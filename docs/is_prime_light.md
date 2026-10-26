---
title: 素数判定（軽量実装）
documentation_of: //math/is_prime_light.hpp
library_category: math/number_theory
---

```cpp
#include "lib/math/is_prime_light.hpp"

bool a = light_is_prime(97); // true
bool b = light_is_prime(1); // false
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `light_is_prime(n)` | `std::uint64_t` 型の `n` が素数なら `true`、それ以外は `false` を返す | $$O(\log n)$$ |

64 bit 符号なし整数の全域を決定的に判定します。0 と 1 は `false`、2 は `true` です。`is_prime.hpp` と同時に include できます。

ある上限までの素数を全て列挙する場合は[素数篩](prime_sieve.md)を使えます。

時間は $$O(\log n)$$、追加空間は $$O(1)$$ です。
