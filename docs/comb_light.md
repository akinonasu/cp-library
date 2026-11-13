---
title: 軽量な階乗・逆階乗・逆元と二項係数
documentation_of: //math/comb_light.hpp
library_category: math/combinatorics
redirect_from:
- /math/combinatorics/comb_light.hpp.html
- /lightweight/math/combinatorics/comb_light.hpp.html
---

```cpp
#define COMB_MAX 500000
#include "lib/math/comb_light.hpp"
#include <cassert>

int main() {
	assert(comb(10, 3).val() == 120);
	assert(fact[5] * finv[5] == mint(1));
	assert(mint(5) * inv[5] == mint(1));
}
```

通常版と同じ `comb`・`Comb<Mint>`・`fact`・`finv`・`inv` を使います。同じ機能は通常版か軽量版のいずれか一方を include してください。負の添字は 0 を返し、`COMB_MAX` の既定値は 10000000 です。

各 modint 型の最初の有効な問い合わせで階乗・逆階乗・逆元を生成し、その型の全操作で表を共有します。使わない型や、負の添字・範囲外の `k`・`inv[0]` に対して 0 を返すだけの問い合わせでは前計算しません。`main` より前の呼び出しにも対応します。既定の `mint` も同じ方式です。

| 操作 | 内容 |
| --- | --- |
| `comb(n,k)` / `comb<Mint>(n,k)` | 二項係数。`n < 0`・`k < 0`・`k > n` なら 0 |
| `Comb<Mint>::comb(n,k)` | 型を指定した二項係数 |
| `fact[i]` / `finv[i]` | 既定の `mint` の階乗／逆階乗。`i < 0` なら 0 |
| `inv[v]` | 整数 `v` の逆元を既定の `mint` で返す。`v <= 0` なら 0 |
| `Comb<Mint>::fact[i]` / `Comb<Mint>::finv[i]` | 指定型の階乗／逆階乗。`i < 0` なら 0 |
| `Comb<Mint>::inv[v]` | 整数 `v` の逆元を指定した `Mint` で返す。`v <= 0` なら 0 |

正の `v` に対して、`inv[v]` は `mint(v) * inv[v] == mint(1)` を満たす値を返します。0 の逆元は定義されませんが、便宜上 `inv[0]` は 0 を返します。`fact`・`finv`・`inv` は読み取り用オブジェクトで、添字は `int`、返り値は modint の値です。配列やポインタとしての受け渡し・値の書き換えには対応しません。

`Mint` は素数法で、整数からの構築、乗算、`mod()`、`inv()` に対応する型にします。`atcoder::modint1000000007` なども使え、ACL 自体は依存に含みません。非負の `i`・`n` と、正の `v` は `min(COMB_MAX,Mint::mod()-1)` 以下にします。既定の法は 998244353 です。`light_combinatorics::inv[v]`・`light_combinatorics::Comb<Mint>::inv[v]` の書き方も使えます。法を変更できる型は、変更後の最初の有効な問い合わせで表を作り直します。

上限を $$N=\min(\texttt{COMB\_MAX},p-1)$$、法を $$p$$ とすると、型ごとの初回前計算は $$O(N+\log p)$$ 時間・$$O(N)$$ 空間、その後の問い合わせは $$O(1)$$ 時間・追加空間です。逆元計算を $$O(\log p)$$ とします。
