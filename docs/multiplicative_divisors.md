---
title: 約数ごとの乗法的関数の値
documentation_of: //math/number_theory/multiplicative_divisors.hpp
---

正整数 `n` の全ての正の約数 `d` について、`{d, f(d)}` を列挙します。`f` は $$f(1)=1$$ で、互いに素な $$a,b$$ に対して $$f(ab)=f(a)f(b)$$ を満たす乗法的関数です。各素数冪での値を渡すだけで利用できます。

メビウス関数は、素数冪に対して $$\mu(p)=-1$$、$$e\ge 2$$ なら $$\mu(p^e)=0$$ と指定します。

```cpp
#include "lib/math/number_theory/multiplicative_divisors.hpp"
#include <iostream>

int main() {
	auto values = multiplicative_divisors(12,
		[](std::uint64_t, int e, std::uint64_t) -> int {
			return e == 1 ? -1 : 0;
		});
	for (auto [d, mu] : values) std::cout << d << ' ' << mu << '\n';
}
```

```text
1 1
2 -1
3 -1
4 0
6 1
12 0
```

素因数分解の結果がある場合は、そのまま渡せます。次の例はオイラー関数の値を列挙します。

```cpp
#include "lib/math/number_theory/multiplicative_divisors.hpp"
#include <cassert>

int main() {
	auto factors = factorize_count(12);
	auto values = multiplicative_divisors(factors,
		[](std::uint64_t p, int, std::uint64_t pe) -> std::uint64_t {
			return pe - pe / p;
		});
	assert(values.back().first == 12 && values.back().second == 4);
}
```

| 関数 | 内容・返り値 |
| --- | --- |
| `multiplicative_divisors(n, prime_power, sorted = true)` | `n` を素因数分解し、全ての `{d, f(d)}` を `std::vector<std::pair<std::uint64_t, T>>` で返す |
| `multiplicative_divisors(factors, prime_power, sorted = true)` | `{素数, 指数}` の列から同じ結果を返す |

`n` は `std::uint64_t` で、`n >= 1` とします。`factors` は `std::vector<std::pair<std::uint64_t, int>>` で、異なる素数と正の指数を渡します。素数の順序は任意で、その積は `std::uint64_t` に収まる必要があります。素数判定や分解結果の正当性の検査は行いません。

`prime_power(p, e, pe)` の引数は順に、素数 `p` (`std::uint64_t`)、指数 `e` (`int`)、素数冪 `pe = p^e` (`std::uint64_t`) です。返り値として `f(p^e)` を返してください。分解結果の各組 `(p, exponent)` について、指数 `1 <= e <= exponent` ごとに 1 回ずつ呼び出します。`e = 0` では呼び出しません。

`T` はコールバックの返り値から参照・`const` を除いた型です。`T(1)` が乗法の単位元であり、コピーと乗算 `a * b`、乗算結果からの `T` の構築ができる必要があります。整数や modint を利用できます。整数の値と演算途中の結果は、選んだ型に収まる必要があります。

`sorted = true` なら約数の昇順、`false` なら順序を指定せず列挙します。値が 0 の約数も含め、各約数を 1 回ずつ返します。`n = 1`、または `factors` が空なら `{1, T(1)}` のみを返し、コールバックは呼び出しません。

| 関数の例 | `prime_power(p, e, pe)` が返す値 |
| --- | --- |
| メビウス関数 | `e == 1 ? -1 : 0` |
| オイラー関数 | `pe - pe / p` |
| 約数の個数 | `e + 1` |
| 約数の和 | $$\displaystyle\sum_{i=0}^{e}p^i$$ |

約数の個数を $$D$$、素因数の指数の合計を $$E$$ とします。コールバック・値のコピー・乗算を $$O(1)$$ とした場合、分解結果を渡す版は昇順で $$O(D\log D)$$ 時間、`sorted = false` で $$O(D)$$ 時間、返り値を含めて $$O(D)$$ 空間です。コールバックは合計 $$E$$ 回呼び出します。整数を渡す版は、さらに素因数分解の期待 $$O(n^{1/4}\log n)$$ 時間、$$O(\log n)$$ 空間を使います。`n = 1` は定数時間・空間です。
