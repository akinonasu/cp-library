---
title: mint の積和部品
documentation_of: //components/mint_inner_product.hpp
---

既定の `mint` の配列の内積を求める内部部品です。`mint_detail` 名前空間で使い、AVX2 が利用できる環境では長い積和を SIMD で計算します。それ以外の環境にも対応します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `inner_product<Reverse=false>(a,b,size)` | `a[i] * b[i]` の和を `mint` で返す。`Reverse=true` では `b[-i]` を使う | $$O(n)$$ |
| `inner_product_scalar<Reverse>(a,b,size)` | 内部表現の積の和を剰余を取らず `__uint128_t` で返す | $$O(n)$$ |
| `inner_product_has_avx2()` | AVX2 が利用できるかを `bool` で返す。x86 向け内部関数 | $$O(1)$$ |
| `inner_product_avx2<Reverse>(a,b,size)` | 内部表現の積の和を SIMD で求め、`__uint128_t` で返す。x86 向け内部関数 | $$O(n)$$ |

`a`・`b` は `const mint*`、`size` は非負の `int` です。`a[0]` から `a[size-1]` まで、通常は `b[0]` から `b[size-1]` まで、逆順では `b[-size+1]` から `b[0]` まで読み取れるようにします。アラインメントの指定は不要です。`size=0` の答えは 0 です。

内部表現 `v` は `0 <= v < 2 * mint::mod()` の範囲で指定します。AVX2 の内部関数は GCC・Clang の x86 向けに定義され、直接呼ぶ場合は AVX2 が利用できる必要があります。通常は自動判定する `inner_product` を使ってください。

要素数を $$n$$ とすると時間は $$O(n)$$、追加空間は $$O(1)$$ です。
