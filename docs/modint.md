---
title: 任意の法の modint
documentation_of: //math/modint.hpp
---

```cpp
#include "lib/math/modint.hpp"
#include <cassert>

int main() {
	using Mint = StaticModint<1000000007>;
	Mint a = -2;
	assert((a * a).val() == 4);
	DynamicModint<>::set_mod(12);
	DynamicModint<> b = 5;
	assert(b.inv().val() == 5);
}
```

`StaticModint<Mod>` はコンパイル時に法を固定し、`DynamicModint<Id = 0>` は実行時に法を設定します。法は `std::uint32_t` の正の値で、素数・合成数の両方に対応します。動的版の初期の法は 998244353 です。`Id` が異なる型は法を独立に保持します。

| 操作 | 内容・返り値 |
| --- | --- |
| `Mint()` / `Mint(x)` | 0／整数を法で正規化して構築。符号付き・符号なし 128 bit 整数まで対応 |
| `Mint::mod()` | 法を `std::uint32_t` で返す |
| `DynamicModint<Id>::set_mod(m)` | その `Id` の法を変更。`m >= 1` |
| `Mint::raw(x)` | `0 <= x < mod()` の `std::uint32_t` を正規化せず格納 |
| `a.val()` | `0 <= value < mod()` の `std::uint32_t` を返す |
| `+a` / `-a` / `a+b` / `a-b` / `a*b` / `a/b` | 同じ型の modint の演算。整数との混合演算も可能 |
| `+=` / `-=` / `*=` / `/=` | 演算して自身への参照を返す |
| `++a` / `--a` / `a++` / `a--` | 1 の加算・減算 |
| `==` / `!=` | 正規化された値を比較 |
| `a.pow(n)` | 非負の `std::int64_t` 指数による累乗。同じ modint 型を返す。0 の 0 乗は 1 を法で正規化した値 |
| `a.inv()` | 逆元を同じ型で返す。法と互いに素でないとき `std::domain_error` |
| `>>` / `<<` | `std::int64_t` の入力／正規化された値の出力 |

割り算は除数の逆元を掛けます。法 1 のときは全ての値が 0 で、逆元も 0 とします。負の指数は `std::invalid_argument` です。動的版は、値を利用している間に法を変更しないでください。法の設定変更を別スレッドと同時に行うことはできません。

`StaticModint` の構築・加減乗算・比較・累乗は定数式でも利用できます。既定の高速な `mint` は [modint998244353](mint.md) を使ってください。`StaticModint` は静的 modint を受け取る畳み込みなどにも渡せます。動的版は、法をコンパイル時定数として要求する API には渡せません。

法を $$m$$ とすると、構築・加減乗算・比較・値の取得・法の設定は $$O(1)$$、逆元・除算は $$O(\log m)$$、累乗は $$O(\log n)$$ 時間です。指数 0 は定数時間です。各値の空間は $$O(1)$$ です。
