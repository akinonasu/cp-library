---
title: 階乗・逆階乗と二項係数
documentation_of: //math/combinatorics/comb.hpp
redirect_from:
- /math/comb.hpp.html
---

```cpp
#define COMB_MAX 500000
#include "lib/math/combinatorics/comb.hpp"

int main() {
	assert(comb(10, 3).val() == 120);
	assert(fact[5].val() == 120);
	assert(fact[5] * finv[5] == mint(1));
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| 自動前計算 | include により、起動時に `fact`・`finv` を準備 | $$O(N)$$ |
| `init_comb()` | `fact`・`finv` を再計算。通常は呼び出し不要 | $$O(N)$$ |
| `comb(n,k)` | 二項係数。`n < 0`、`k < 0`、`n < k` なら 0 | $$O(1)$$ |
| `fact[n]` / `finv[n]` | 階乗／逆階乗の前計算値 | $$O(1)$$ |

`init_comb()` を呼ばずに使えます。include すると起動時に上限まで自動で前計算します。`fact[n]` は $$n!$$、`finv[n]` はその逆数です。mod は 998244353 です。

上限までの `comb`・`fact`・`finv` をそのまま何度でも使えます。返り値は `mint` なので、通常の整数として取り出す場合は `comb(n,k).val()` とします。`fact`・`finv` はこれまでと同じ C 配列です。

既存の `init_comb()` も使えますが、自動前計算に加えて再計算するため、通常は削除して構いません。配列の値を書き換えた後に元へ戻したい場合などに使います。

FPS と階乗・逆階乗の生成処理を共有します。256 要素以上の AVX2 対応 x86 では SIMD、それ以外ではスカラー処理を使います。

`COMB_MAX` は include 前に指定します。省略時は 502022 です。$$n$$ は前計算範囲内とし、`COMB_MAX` は 0 以上 998244353 未満が必要です。複数の cpp から include する場合は、全て同じ `COMB_MAX` を指定してください。初回の自動前計算はプログラム全体で 1 回です。

`COMB_MAX` を $$N$$ として、起動時に $$O(N)$$ 時間・空間を使います。`init_comb()` の再計算も $$O(N)$$ 時間、`comb(n,k)` と配列の参照は $$O(1)$$ です。include した場合は、二項係数を使わなくても起動時の前計算が発生します。
