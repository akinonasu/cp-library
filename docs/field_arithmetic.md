---
title: 体の内部演算
documentation_of: //components/field_arithmetic.hpp
---

行列と線形漸化式の復元で共有する、体の演算と連続配列の計算です。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `field_detail::Field<T>` | 通常の型 `T` を使う演算 | — |
| `field_detail::ModularField<T>` | 固定法の modint を `uint32_t` の標準表現で扱う演算 | — |
| `Ops::add(a,b)` / `Ops::mul(a,b)` / `Ops::neg(a)` / `Ops::inv(a)` | 加算・乗算・符号反転・逆元 | 型 `T` の演算による |
| `Ops::dot(a,b,n)` | 長さ `n` の内積 | $$O(n)$$ |
| `Ops::scale(a,n,x)` | `a` の長さ `n` の領域を `x` 倍 | $$O(n)$$ |
| `Ops::axpy(a,b,n,x)` | `a` の長さ `n` の領域に `x*b` を加える | $$O(n)$$ |

`Ops::V` が配列の要素型です。`ModularField<T>` は固定法が $$2\le p<2^{31}$$ の modint 用で、`val()` と `raw()` に対応する型を使います。逆元を求める値は可逆である必要があります。

AVX2 は対応する CPU でのみ使用し、非対応環境では通常の計算に切り替えます。`FIELD_DISABLE_AVX2` を定義すると AVX2 を無効にできます。

体の演算を $$O(1)$$ として、配列の各操作は時間 $$O(n)$$、追加空間は $$O(1)$$ です。
