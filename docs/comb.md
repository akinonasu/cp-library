---
title: 階乗・逆階乗と二項係数
documentation_of: //math/comb.hpp
redirect_from:
- /math/combinatorics/comb.hpp.html
- /lightweight/math/combinatorics/comb.hpp.html
library_category: math/combinatorics
---

```cpp
#define COMB_MAX 500000
#include "lib/math/comb.hpp"

int main() {
	assert(comb(10, 3).val() == 120);
	assert(fact[5].val() == 120);
	assert(fact[5] * finv[5] == mint(1));
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| 自動前計算 | include により、起動時に `fact`・`finv` を準備 | $$O(N)$$ |
| `comb(n,k)` | 二項係数。`n < 0`、`k < 0`、`n < k` なら 0 | $$O(1)$$ |
| `fact[i]` / `finv[i]` | 階乗／逆階乗。`i < 0` なら 0 | $$O(1)$$ |
| `comb<Mint>(n,k)` / `Comb<Mint>::comb(n,k)` | 指定した型の二項係数 | 初回前計算後 $$O(1)$$ |
| `Comb<Mint>::fact[i]` / `Comb<Mint>::finv[i]` | 指定した型の階乗／逆階乗。`i < 0` なら 0 | 初回前計算後 $$O(1)$$ |

include すると起動時に上限まで自動で前計算します。`fact[n]` は $$n!$$、`finv[n]` はその逆数です。mod は 998244353 です。

上限までの `comb`・`fact`・`finv` を何度でも使えます。返り値は `mint` なので、通常の整数として取り出す場合は `comb(n,k).val()` とします。`fact`・`finv` は `operator[]` を持つ読み取り用オブジェクトで、値をコピーして返します。負の添字は 0 を返します。C 配列やポインタとしての受け渡し・テーブルへの代入には対応しません。

既定の `mint` では、256 要素以上の AVX2 対応 x86 で SIMD を使います。

modint 型を一度指定して `comb(n,k)`・`fact[i]`・`finv[i]` の書き方を使いたい場合は、[modint を指定する二項係数](comb_mod.md) を使ってください。

他の modint は型を指定します。`atcoder::modint1000000007`・`atcoder::modint998244353`・`atcoder::dynamic_modint` などを使えます。ACL はこのヘッダー自体の依存には含まれません。

```cpp
#define COMB_MAX 500000
#include <atcoder/modint>
#include "lib/math/comb.hpp"

int main() {
	using M = atcoder::modint1000000007;
	using C = Comb<M>;
	assert(comb<M>(10, 3).val() == 120);
	assert(C::comb(10, 3).val() == 120);
	assert(C::fact[5].val() == 120);
	assert(C::fact[5] * C::finv[5] == M(1));
	assert(C::fact[-1] == M(0) && C::finv[-1] == M(0));
	const auto &fact = C::fact;
	const auto &finv = C::finv;
	assert(fact[5] * finv[5] == M(1));
}
```

既定以外の型では、最初の有効な `comb`・`fact`・`finv` の参照時にだけ前計算します。同じ型では 3 つの操作がテーブルを共有し、複数の cpp から使ってもプログラム全体で 1 つです。使わない型や、負の添字・範囲外の `k` に対して 0 を返すだけの問い合わせでは、テーブルを生成しません。`Comb<mint>` は起動時に生成する既定のテーブルを再利用します。

modint は素数法で、整数からの構築・乗算・`mod()`・`inv()` に対応するものを指定します。別型の前計算上限は `min(COMB_INF,Mint::mod()-1)` です。有効な問い合わせの非負の添字 `i`・`n` はこの上限以下にしてください。合成数法や `n >= mod` の二項係数には [任意 mod の二項係数](binomial_mod.md) を使ってください。

整数値を返す `val()` を持つ modint は、32 bit 以下の法で前計算を高速化します。`val()` はその値の整数表現を返すものとし、負の代表元にも対応します。`val()` を持たない型も利用できます。

`dynamic_modint` のように法を変更できる型では、法が変わった後の最初の有効なアクセスでテーブルを作り直します。異なる型は、同じ法でも別のテーブルを使います。

`COMB_MAX` は include 前に指定します。省略時は 10000000 です。$$n$$ は前計算範囲内とし、`COMB_MAX` は 0 以上 998244353 未満が必要です。複数の cpp から include する場合は、全て同じ `COMB_MAX` を指定してください。初回の自動前計算はプログラム全体で 1 回です。既定の階乗・逆階乗の領域は合計 80000008 バイト（約 80 MB）です。

`COMB_MAX` を $$N$$ として、起動時に $$O(N)$$ 時間・空間を使います。`comb(n,k)` と配列の参照は $$O(1)$$ です。include した場合は、二項係数を使わなくても起動時の前計算が発生します。

別型の実際の上限を $$L=\min(N,p-1)$$、法を $$p$$ とすると、その型の初回前計算は $$O(L+\log p)$$ 時間・$$O(L)$$ 空間です。乗算を $$O(1)$$、逆元計算を $$O(\log p)$$ とします。その後の問い合わせは $$O(1)$$ です。法を変更した場合は同じ前計算コストが再びかかります。
