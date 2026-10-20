---
title: 二項係数の prefix sum（差分更新）
documentation_of: //math/combinatorics/binomial_prefix_sum_incremental.hpp
---

$$\displaystyle S(n,k)=\sum_{i=0}^{k-1}\binom ni$$ を、直前のクエリから差分更新して求めます。右端を含みません。クエリは呼び出した順に処理します。

```cpp
#include "lib/math/combinatorics/binomial_prefix_sum_incremental.hpp"

IncrementalBinomialPrefixSum<> sum(500000);
auto a = sum.query(5, 3); // 16
auto b = sum.query(6, 3); // 22
auto c = sum.query(6, 4); // 42
```

`Mint` の既定は `mint` です。素数 mod の `mod()`・`inv()` と整数からの構築・加減乗算に対応する modint を指定できます。$$0\leq n\leq\text{max\_n}<\min(\text{mod},\text{INT\_MAX})$$ とし、使用中に法を変更しないでください。`k` は `std::int64_t` で、$$[0,n+1]$$ に収めて扱います。

`build(max_n)` は前計算と状態を作り直し、`max_n()` は前計算の上限を返します。初期状態は $$(n,k)=(0,0)$$ です。同じクエリは $$O(1)$$、離れたクエリほど更新量が増えます。

$$N=\text{max\_n}$$ として、構築・`build` は $$O(N)$$ 時間・空間と逆元計算1回、`max_n()` は $$O(1)$$ です。

`query` は直前の状態との差を $$\Delta n,\Delta k$$ として $$O(\lvert\Delta n\rvert+\lvert\Delta k\rvert)$$ 時間・$$O(1)$$ 追加空間です。

クエリ数を $$Q$$、差分の総和を $$D$$ とすると全体は $$O(N+Q+D)$$ 時間です。
