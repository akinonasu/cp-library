---
title: 素数篩
documentation_of: //math/number_theory/prime_sieve.hpp
redirect_from:
- /math/prime_sieve.hpp.html
---

```cpp
#include "lib/math/number_theory/prime_sieve.hpp"

auto primes = prime_sieve(10); // {2,3,5,7}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `prime_sieve(n)` | `n` 未満の素数を昇順の `vector<std::size_t>` で返す | $$O(n\log\log n)$$ |
| `for_each_prime(n, f)` | `n` 未満の各素数 `p` に対して `f(p)` を昇順に呼ぶ。返り値は `void` | $$O(n\log\log n)$$ |

非負の上限 $$n$$ を渡すと、$$n$$ 未満の素数を昇順の `std::vector<std::size_t>` で返します。上限そのものは含みません。`prime_sieve(2)` は空配列、`prime_sieve(3)` は `{2}` です。返された配列を `for (auto p : primes)` で走査できます。

`for_each_prime` の `f` は `std::size_t` を 1 個受け取る関数です。素数の配列を保持せずに集計・出力したいときに使えます。`n <= 2` では一度も呼び出しません。

```cpp
std::size_t count = 0, sum = 0;
for_each_prime(10, [&](std::size_t p) { ++count; sum += p; });
// count = 4, sum = 17
```

小さい入力は奇数だけを一括で篩います。大きい入力は 2・3・5 と互いに素な候補を 1 bit ずつ保持し、区間ごとに篩います。7・11・13 の倍数を周期パターンで除き、残りの素数の次の倍数を区間間で引き継ぎます。

時間は $$O(n\log\log n)$$、補助空間は $$\displaystyle O(\sqrt n)$$ です。`prime_sieve` は返り値の配列に追加で $$O(\pi(n))$$ の空間を使います。
