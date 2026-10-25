---
title: 素因数分解・素数判定・約数列挙
documentation_of: //math/number_theory/factorize.hpp
redirect_from:
- /math/factorize.hpp.html
---

```cpp
#include "lib/math/number_theory/factorize.hpp"

auto factors = factorize(360); // {2,2,2,3,3,5}
auto counts = factorize_count(360); // { {2,3},{3,2},{5,1} }
bool prime = is_prime(998244353); // true
auto ds = divisors(12); // {1,2,3,4,6,12}
auto unsorted = divisors(12, false); // 順序を指定せず列挙
auto reused = divisors(counts); // 360 の約数。再度の素因数分解は不要
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `is_prime(n)` | 素数かを判定 | $$O(\log n)$$ |
| `factorize(n)` | 素因数を重複込み・昇順で返す。`n >= 1` | 期待 $$O(n^{1/4}\log n)$$ |
| `factorize_count(n)` | `{素因数, 指数}` の昇順列を返す。`n >= 1` | 期待 $$O(n^{1/4}\log n)$$ |
| `divisors(n, sorted = true)` | 正の約数を `vector<uint64_t>` で返す。`n >= 1` | 素因数分解に加え、昇順は $$O(d\log d)$$、`false` は $$O(d)$$ |
| `divisors(factors, sorted = true)` | `factorize_count` 形式の分解結果から正の約数を返す | 昇順は $$O(d\log d)$$、`false` は $$O(d)$$ |

引数は `uint64_t` です。`factorize(n)` は素因数の昇順列、`factorize_count(n)` は `{素因数, 指数}` の昇順列を返します。素因数分解は $$n \ge 1$$ とします。

例えば `counts` の `{2,3}` は素因数 2 の指数が 3、すなわち $$2^3$$ を表します。1 の分解結果はどちらも空配列です。0 と負数の素因数分解は扱いません。前処理を呼ぶ必要はなく、そのまま各関数を使えます。

`divisors(n)` は 1 と `n` を含む全ての正の約数を、重複なしの昇順で返します。`divisors(1)` は `[1]` です。第 2 引数に `false` を渡すとソートを省略します。この場合も全ての約数を 1 回ずつ返しますが、順序は保証しません。0 と負数の約数列挙は扱いません。

`divisors(factors)` の `factors` は `vector<pair<uint64_t,int>>` です。異なる素数と正の指数を渡し、その積は `uint64_t` に収まる必要があります。素数の並び順は問いません。空配列は 1 の分解結果として扱い、`[1]` を返します。渡した分解結果の素数判定や正当性の検査は行いません。

約数と同時にメビウス関数・オイラー関数などの値も列挙する場合は、[約数ごとの乗法的関数の値](multiplicative_divisors.md) を利用できます。

64 bit 整数演算を $$O(1)$$ として、`is_prime(n)` は $$O(\log n)$$ 時間、$$O(1)$$ 空間です。`factorize(n)`・`factorize_count(n)` は Pollard Rho の期待計算量で $$O(n^{1/4} \log n)$$ 時間、$$O(\log n)$$ 空間です。

約数の個数を $$d$$ とすると、`divisors(n)` は期待 $$O(n^{1/4}\log n+d\log d)$$ 時間、`divisors(n,false)` は期待 $$O(n^{1/4}\log n+d)$$ 時間、いずれも返り値を含めて $$O(d+\log n)$$ 空間です。分解結果を渡す版は素因数分解を省き、$$O(d)$$ 空間です。
