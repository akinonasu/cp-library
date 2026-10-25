---
title: 法を指定する階乗・逆階乗と二項係数
documentation_of: //math/combinatorics/comb_mod.hpp
---

法を一度指定し、`comb(n,k)`・`fact[i]`・`finv[i]` をそのまま使います。modint の型を各呼び出しに指定する必要はありません。

```cpp
#define COMB_MOD 1000000007
#define COMB_MAX 500000
#include "lib/math/combinatorics/comb_mod.hpp"
#include <cassert>

int main() {
	assert(comb(10, 3).val() == 120);
	assert(fact[5].val() == 120);
	assert(fact[5] * finv[5] == 1);
	comb_mint answer = comb(100, 20);
	assert(answer == comb(100, 80));
}
```

`COMB_MOD` は素数を指定します。既定値は 1000000007 で、`std::uint32_t` に収まる必要があります。`COMB_MAX` は 0 以上 `INT_MAX` 未満の整数で、既定値は 10000000 です。どちらも include 前に指定します。実際の前計算上限は `min(COMB_MAX, COMB_MOD-1)` です。

| 操作 | 内容・返り値 |
| --- | --- |
| `comb(n,k)` | 二項係数を `comb_mint` で返す。`n, k` は `int`。`n < 0`・`k < 0`・`k > n` なら 0 |
| `fact[i]` | `i!` を `comb_mint` で返す。`i` は `int` で、負なら 0 |
| `finv[i]` | `i!` の逆元を `comb_mint` で返す。`i` は `int` で、負なら 0 |
| `comb_mint` | `StaticModint<COMB_MOD>`。返り値と同じ法で演算するときに使う型 |

有効な問い合わせの `n` と、非負の添字 `i` は前計算上限以下としてください。テーブルは読み取り専用で、添字アクセスは値をコピーして返します。C 配列・ポインタとしての利用や、テーブルの書き換えはできません。

最初の有効な問い合わせで指定した法のテーブルだけを生成し、以降は共有します。0 を返すだけの問い合わせでは生成しません。複数の cpp から include する場合は、`COMB_MOD`・`COMB_MAX` を全て揃えてください。

[既定の二項係数](comb.md) とこのヘッダーは、どちらか一方を選んで使ってください。両方とも同名の `comb`・`fact`・`finv` を提供します。`comb_mod.hpp` では既定の `mint` の表を生成せず、型名 `mint` も変更しません。他のライブラリの mod 998244353 の `mint` と `comb_mint` は異なる型なので、両者の値を直接混ぜて演算しないでください。

合成数法や `n >= COMB_MOD` の二項係数には [任意 mod の二項係数](binomial_mod.md) を使います。

実際の上限を $$L$$、法を $$p$$ とすると、最初の前計算は $$O(L+\log p)$$ 時間・$$O(L)$$ 空間、それ以降の問い合わせは $$O(1)$$ 時間です。負の添字・無効な `k` などによる 0 の返却は、初回から定数時間です。
