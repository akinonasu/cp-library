---
title: modint を指定する階乗・逆階乗と二項係数
documentation_of: //math/comb_mod.hpp
library_category: math/combinatorics
redirect_from:
- /math/combinatorics/comb_mod.hpp.html
- /lightweight/math/combinatorics/comb_mod.hpp.html
---

modint 型を一度指定し、`comb(n,k)`・`fact[i]`・`finv[i]` を使います。

```cpp
#include <atcoder/modint>
#include "lib/math/comb_mod.hpp"
#include <cassert>

int main() {
	auto [comb, fact, finv] = make_comb<atcoder::modint1000000007>(500000);
	assert(comb(10, 3).val() == 120);
	assert(fact[5].val() == 120);
	assert(fact[5] * finv[5] == 1);
}
```

自前の modint も同じように渡せます。次の例はこのライブラリの `StaticModint` を使います。

```cpp
#include "lib/math/modint.hpp"
#include "lib/math/comb_mod.hpp"
#include <cassert>

int main() {
	using Mint = StaticModint<1000000007>;
	auto [comb, fact, finv] = make_comb<Mint>(500000);
	Mint answer = comb(100, 20);
	assert(answer == comb(100, 80));
}
```

| 操作 | 内容・返り値 |
| --- | --- |
| `make_comb<Mint>(max_n = 10000000)` | 同じ表を共有する `comb`・`fact`・`finv` を持つ `CombMod<Mint>` を返す。構造化束縛で取り出して使える |
| `comb(n,k)` | 二項係数を渡した `Mint` 型で返す。`n, k` は `int`。`n < 0`・`k < 0`・`k > n` なら 0 |
| `fact[i]` | `i!` を `Mint` で返す。`i` は `int` で、負なら 0 |
| `finv[i]` | `i!` の逆元を `Mint` で返す。`i` は `int` で、負なら 0 |

`Mint` は素数法の modint とし、`mod()` は 64 bit 以下の整数型で法を返すものとします。`mod()`・`inv()`・整数からの構築・乗算に対応し、既定構築で 0 になる必要があります。`val()` がなくても利用できます。整数値を返す `val()` があり、法が 32 bit 以下の場合は、共通の高速な階乗生成を使います。ACL 自体はこのヘッダーの依存には含まれません。

`max_n` は 0 以上 `INT_MAX` 未満です。実際の上限は `min(max_n, Mint::mod()-1)` で、有効な問い合わせの `n` と、非負の添字 `i` はこの上限以下にしてください。テーブルは読み取り専用で、添字アクセスは値をコピーして返します。C 配列・ポインタとしての利用や、テーブルの書き換えはできません。

最初の有効な問い合わせで指定した型のテーブルだけを生成します。0 を返すだけの問い合わせでは生成しません。一度の `make_comb` で取り出した三つの操作や、それらのコピーは同じ表を共有します。元の返り値を破棄した後も、コピーした操作は使えます。別々に `make_comb` を呼ぶと、それぞれ独立した表を持ちます。

動的 modint では、法が変わった後の最初の有効なアクセスで表を作り直します。新しい法も素数とし、変更前の法で得た値は使い続けないでください。テーブルの初期化や法の変更を伴う同時アクセスには外側で同期が必要です。

[既定の二項係数](comb.md) と同時に include できます。関数内で上のように構造化束縛すると、その関数内では渡した `Mint` の `comb`・`fact`・`finv` を使えます。このヘッダー自体はグローバルな同名の関数や変数を定義せず、既定の `mint` の表も生成しません。

合成数法や `n >= mod` の二項係数には [任意 mod の二項係数](binomial_mod.md) を使います。

実際の上限を $$L$$、法を $$p$$ とすると、`make_comb` は定数時間・空間です。最初の前計算は $$O(L+\log p)$$ 時間・$$O(L)$$ 空間、それ以降の問い合わせは $$O(1)$$ 時間です。法を変えた場合は同じ前計算コストが再びかかります。負の添字・無効な `k` などによる 0 の返却は、初回から定数時間です。
