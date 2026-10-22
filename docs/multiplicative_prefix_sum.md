---
title: 乗法的関数の prefix sum
documentation_of: //math/number_theory/multiplicative_prefix_sum.hpp
---

乗法的関数 $$f$$ の $$\displaystyle F(x)=\sum_{i=1}^x f(i)$$ を計算します。`f(1) = 1` とし、互いに素な $$a,b$$ について $$f(ab)=f(a)f(b)$$ を満たす関数が対象です。上限を含み、`F(0) = 0` です。

例えば、オイラーのトーティエント関数の和は次のように求められます。

```cpp
#include "lib/math/number_theory/multiplicative_prefix_sum.hpp"
#include "lib/math/mint.hpp"

MultiplicativePrefixSum<mint> sieve(100);
auto primes = sieve.prime_sum_linear(mint(-1), mint(1));
auto phi = [](std::uint64_t p, unsigned, std::uint64_t pe) {
    return mint(pe - pe / p);
};

auto answer = sieve.sum(primes, phi); // 3044
auto prefix = sieve.prefix_sum(primes, phi);
auto at10 = sieve.get(prefix, 10); // 32
```

利用には、素数上の prefix sum $$\displaystyle G(x)=\sum_{p\leq x,\ p\text{ prime}} f(p)$$ の表と、素数冪に対する値を返す関数が必要です。`prime_sum_linear(a,b)` は $$f(p)=a+bp$$ の場合の素数上の表を作ります。上の例では $$f(p)=p-1$$ です。引数・返り値は元の整数に対する値で、1 に対する値は内部で加えます。

| 操作 | 内容・返り値 |
| --- | --- |
| `MultiplicativePrefixSum<T>(n = 0)` | 上限 `n` の計算領域を作る |
| `max_n()` / `size()` | 上限／商の表の要素数 |
| `quotients()` | 異なる正の `n/i` を降順に並べた `const vector<uint64_t>&` |
| `index(x)` | `quotients()` に含まれる正の `x` の添字 |
| `prime_sum_linear(a,b)` | 素数上の値が `a+b*p` の場合の表を返す |
| `prime_sum(integer_prefix)` | 完全乗法的な補助関数の素数上の表を返す。条件は後述 |
| `sum(prime_prefix,prime_power)` | `F(n)` を `T` で返す |
| `prefix_sum(prime_prefix,prime_power)` | `quotients()` の各点に対する `F(x)` を `vector<T>` で返す |
| `get(table,x)` | 作った表の `x` に対する値。`x = 0` は 0 |
| `swap(other)` | 二つの計算領域を交換する |

`prime_power(p,e,pe)` は $$f(p^e)$$ を `T` で返します。`p` は素数、`e >= 1`、`pe = p^e <= n` です。同じ引数には同じ値を返してください。`prime_prefix` の素数上の値と、このコールバックの `e = 1` の値は一致する必要があります。高い冪に対する値は任意に指定できます。

`T` は整数・modint など、0・1 の構築と加減乗算を備えた可換環の値を使います。コールバックの戻り値も `T` にしてください。逆元や除算は不要で、合成数 mod の型も使えます。`prime_sum_linear` では `std::uint64_t` から `T` への変換も使います。三角数は整数の片側を先に半分にするため、2 の逆元を要求しません。整数型の場合は、中間値もその型に収めてください。大きな整数和には `__int128_t` などを使えます。

代表的な指定は次の通りです。

| 関数 | 素数上の表 | `prime_power(p,e,pe)` |
| --- | --- | --- |
| 常に 1 | `prime_sum_linear(T(1),T(0))` | `T(1)` |
| 恒等関数 | `prime_sum_linear(T(0),T(1))` | `T(pe)` |
| トーティエント φ | `prime_sum_linear(T(-1),T(1))` | `T(pe-pe/p)` |
| メビウス μ | `prime_sum_linear(T(-1),T(0))` | `e == 1 ? T(-1) : T(0)` |
| 約数個数 τ | `prime_sum_linear(T(2),T(0))` | `T(e+1)` |
| 約数和 σ | `prime_sum_linear(T(1),T(1))` | $$\displaystyle\sum_{j=0}^{e}T(p)^j$$ |
| 平方数なら 1 | `prime_sum_linear(T(0),T(0))` | `T(e%2 == 0)` |

`prime_sum(integer_prefix)` では、補助関数 $$g$$ が完全乗法的、すなわち互いに素という制約なしに $$g(ab)=g(a)g(b)$$、かつ $$g(1)=1$$ であることが必要です。`integer_prefix(x)` に $$\displaystyle\sum_{i=1}^x g(i)$$ を与えると、各点で $$\displaystyle\sum_{p\leq x,\ p\text{ prime}}g(p)$$ を返します。`g(i)=1,i,i^2` や Dirichlet 指標などが使えます。返された表の線形結合を作れば、素数上の値が多項式などで表せる関数にも対応できます。別の方法で作った正しい素数上の表を `sum`・`prefix_sum` に渡しても構いません。

`n` は非負の `std::uint64_t` です。全ての表は `quotients()` と同じ長さ・順序です。`get`・`index` の正の引数はこの列に含まれる値だけに限ります。特に 1〜整数平方根までは全て含まれます。`prefix_sum` は長さ `n` の配列を作る操作ではありません。任意の上限 `m` の和は、上限 `m` のオブジェクトを構築して `sum` で求められます。

上限 0 では表は空で、`sum` と `get(table,0)` は 0 です。上限 1 では `sum` は 1 です。呼び出しはオブジェクトや入力の表を変更しないため、別の関数や異なる素数冪の値で計算領域を再利用できます。コピーは独立し、ムーブ後の元は上限 0 の状態になります。必要な領域は入力に応じて増えるため、時間とメモリに収まる範囲で使ってください。

商の表と素数篩を共有し、素数上の表は Min_25 篩で作ります。小さい引数は商が同じになる範囲をまとめ、商の添字を使って除算を減らします。`sum` は Black algorithm の列挙を使い、残りが素数と素数の二つの積に限られる部分をまとめます。`prefix_sum` は逆順の素数から高い冪を復元する Min_25 篩を使います。

`T` の演算とコールバックを $$O(1)$$ とすると、構築は $$\displaystyle O(\sqrt n\log\log n)$$ 時間・$$\displaystyle O(\sqrt n)$$ 空間です。`prime_sum`・`prime_sum_linear`・`sum`・`prefix_sum` はそれぞれ $$\displaystyle O\!\left(\frac{n^{3/4}}{\log n}\right)$$ 時間、保持する表と作業領域は $$\displaystyle O(\sqrt n)$$ です。表の参照は $$O(1)$$、コピーは $$\displaystyle O(\sqrt n)$$ 時間・空間です。`n <= 1` は定数時間で処理します。
