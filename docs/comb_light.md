---
title: 軽量な階乗・逆階乗と二項係数
documentation_of: //math/comb_light.hpp
library_category: math/combinatorics
redirect_from:
- /math/combinatorics/comb_light.hpp.html
- /lightweight/math/combinatorics/comb_light.hpp.html
---

```cpp
#define COMB_MAX 500000
#include "lib/math/comb_light.hpp"

int main() {
	using light_combinatorics::comb, light_combinatorics::fact, light_combinatorics::finv;
	assert(comb(10, 3).val() == 120);
	assert(fact[5] * finv[5] == mint(1));
}
```

`light_combinatorics` 名前空間の `comb`・`Comb<Mint>`・`fact`・`finv` を使います。通常版と同じ添字形式で、負の添字は 0 を返します。`COMB_MAX` の既定値は 10000000 です。通常版とは独立した実装で、同時に include できます。

各 modint 型の最初の有効な問い合わせで階乗・逆階乗を生成し、その型の全操作で表を共有します。使わない型、負の添字、範囲外の `k` を 0 とする問い合わせでは前計算しません。`main` より前の呼び出しにも対応します。既定の `mint` も同じ方式です。通常版 FPS や、その SIMD カーネルは読み込みません。

| 操作 | 内容 |
| --- | --- |
| `comb(n,k)` / `comb<Mint>(n,k)` | 二項係数。`n < 0`・`k < 0`・`k > n` なら 0 |
| `Comb<Mint>::comb(n,k)` | 型を指定した二項係数 |
| `fact[i]` / `finv[i]` | 既定の `mint` の階乗／逆階乗。`i < 0` なら 0 |
| `Comb<Mint>::fact[i]` / `Comb<Mint>::finv[i]` | 指定型の階乗／逆階乗。`i < 0` なら 0 |

`Mint` は素数法で、整数からの構築、乗算、`mod()`、`inv()` に対応する型にします。`atcoder::modint1000000007` なども使え、ACL 自体は依存に含みません。非負の `i`・`n` は `min(COMB_MAX,Mint::mod()-1)` 以下にします。法を変更できる型は、変更後の最初の有効な問い合わせで表を作り直します。

上限を $$N=\min(\texttt{COMB\_MAX},p-1)$$、法を $$p$$ とすると、型ごとの初回前計算は $$O(N+\log p)$$ 時間・$$O(N)$$ 空間、その後の問い合わせは $$O(1)$$ 時間・追加空間です。逆元計算を $$O(\log p)$$ とします。
