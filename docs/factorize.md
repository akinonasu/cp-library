---
title: "素因数分解・素数判定"
documentation_of: //math/factorize.hpp
---

```cpp
#include "lib/math/factorize.hpp"

auto factors = factorize(360); // {2,2,2,3,3,5}
auto counts = factorize_count(360); // { {2,3},{3,2},{5,1} }
bool prime = is_prime(998244353); // true
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `is_prime(n)` | 素数かを判定 | $$O(\log n)$$ |
| `factorize(n)` | 素因数を重複込み・昇順で返す。`n >= 1` | 期待 $$O(n^{1/4}\log n)$$ |
| `factorize_count(n)` | `{素因数, 指数}` の昇順列を返す。`n >= 1` | 期待 $$O(n^{1/4}\log n)$$ |

引数は `uint64_t` です。`factorize(n)` は素因数の昇順列、`factorize_count(n)` は `{素因数, 指数}` の昇順列を返します。素因数分解は $$n \ge 1$$ とします。

例えば `counts` の `{2,3}` は素因数 2 の指数が 3、すなわち $$2^3$$ を表します。1 の分解結果はどちらも空配列です。0 と負数の素因数分解は扱いません。前処理を呼ぶ必要はなく、そのまま各関数を使えます。

64 bit 整数演算を $$O(1)$$ として、`is_prime(n)` は $$O(\log n)$$ 時間、$$O(1)$$ 空間です。`factorize(n)`・`factorize_count(n)` は Pollard Rho の期待計算量で $$O(n^{1/4} \log n)$$ 時間、$$O(\log n)$$ 空間です。
