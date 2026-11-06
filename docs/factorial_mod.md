---
title: Factorial mod 998244353
documentation_of: //math/factorial_mod.hpp
library_category: math/combinatorics
---

大きな非負整数 $$N$$ に対して、$$N! \bmod 998244353$$ を求めます。法を固定して用意した小さな表から最寄りの階乗を取り出し、短い区間の積で補正します。実行時の前計算は不要です。

```cpp
#include "lib/math/factorial_mod.hpp"
#include <cassert>
#include <cstdint>

int main() {
	assert(factorial_mod(0).val() == 1);
	assert(factorial_mod(10).val() == 3628800);
	assert(factorial_mod(998244352).val() == 998244352);
	assert(factorial_mod(998244353).val() == 0);
	assert(factorial_mod(std::uint64_t(1) << 60).val() == 0);
}
```

`factorial_mod(std::uint64_t n)` は、剰余を既存の `mint` 型で返します。整数値は `.val()` で取得できます。引数に既定値はなく、負の値は渡さないでください。`std::uint64_t` の全域を受け付け、`n >= 998244353` なら即座に 0 を返します。`0!` は 1 です。別の法や動的法には対応しません。

保持する表は `components/factorial_mod_table.hpp` の 15233 個の `uint32_t` で、`table[k]` は `(65536*k)! mod 998244353` です。最後の項は `(998244352)!` で、値は `998244352` になります。表は定数として約 60 KiB を使用し、起動時の生成やヒープ領域の確保はありません。

表の間隔を $$B=65536$$、法を $$p=998244353$$ とすると、`n < p` の計算時間は $$O(B+\log p)$$、最大 32768 個の整数の積と、必要なら 1 回の逆元計算です。追加空間は $$O(1)$$、固定表の保持は $$O(p/B)$$ です。`n >= p` と表に一致する値は定数時間です。多数の連続した小さい添字に対する階乗・逆階乗の全表が必要な場合は、`Comb` の階乗表も利用できます。
