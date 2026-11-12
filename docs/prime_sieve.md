---
title: 素数篩
documentation_of: //math/prime_sieve.hpp
redirect_from:
- /math/number_theory/prime_sieve.hpp.html
- /lightweight/math/number_theory/prime_sieve.hpp.html
library_category: math/number_theory
---

```cpp
#include "lib/math/prime_sieve.hpp"

auto primes = prime_sieve(10); // {2,3,5,7}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `prime_sieve(n)` | `n` 未満の素数を昇順の `vector<std::size_t>` で返す | $$O(n\log\log n)$$ |
| `for_each_prime(n, f)` | `n` 未満の各素数 `p` に対して `f(p)` を昇順に呼ぶ。返り値は `void` | $$O(n\log\log n)$$ |
| `prime_sieve_sample(n,stride,offset = 0)` | 素数の総数と、指定した順位の素数だけを選んだ配列を `pair<size_t,vector<size_t>>` で返す | $$O(n\log\log n)$$ |

非負の上限 $$n$$ を渡すと、$$n$$ 未満の素数を昇順の `std::vector<std::size_t>` で返します。上限そのものは含みません。`prime_sieve(2)` は空配列、`prime_sieve(3)` は `{2}` です。返された配列を `for (auto p : primes)` で走査できます。

`for_each_prime` の `f` は `std::size_t` を 1 個受け取る関数です。素数の配列を保持せずに集計・出力したいときに使えます。`n <= 2` では一度も呼び出しません。

```cpp
std::size_t count = 0, sum = 0;
for_each_prime(10, [&](std::size_t p) { ++count; sum += p; });
// count = 4, sum = 17
```

`prime_sieve_sample` は、`n` 未満の素数を 0 始まりで数え、`offset` 番目から順位を `stride` ずつ進めて選びます。つまり、順位 `offset`, `offset+stride`, `offset+2*stride`, ... の素数を取り出します。引数は全て `std::size_t` で、`stride > 0` が必要です。返り値の最初は選ぶ前の素数の総数、2 番目は選んだ素数を昇順に並べた配列です。`offset` が素数の総数以上なら配列は空です。`n <= 2` なら総数も 0 になります。

```cpp
#include "lib/math/prime_sieve.hpp"
#include <cassert>
#include <cstddef>
#include <vector>

int main() {
	auto [count, selected] = prime_sieve_sample(20, 2, 1);
	assert(count == 8); // 2, 3, 5, 7, 11, 13, 17, 19 の 8 個
	assert((selected == std::vector<std::size_t>{3, 7, 13, 19}));
}
```

時間は $$O(n\log\log n)$$、補助空間は $$\displaystyle O(\sqrt n)$$ です。`prime_sieve` は返り値の配列に追加で $$O(\pi(n))$$ の空間を使います。ここで $$\pi(n)$$ は $$n$$ 以下の素数の個数です。
`prime_sieve_sample` は、選んだ素数の個数を $$k$$ として、返り値に追加で $$O(k)$$ の空間を使います。
