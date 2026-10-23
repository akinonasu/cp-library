---
title: 多項式補間の SIMD 処理
documentation_of: //components/fps_interpolation.hpp
---

`fps_detail::evaluation_poly` の補間用の内部部品です。通常は [補間](interpolate.md) を使います。Linux x86_64 / GCC で定義され、AVX2 が必要です。

`native_interpolate(points,values)` は `mint` の評価点と値から係数列の `vector<mint>` を返します。点数は非零、両配列の長さは等しく、評価点は相異なる必要があります。生ワードを扱う `interpolate` と必要な作業領域を返す `interpolate_words` の前提条件は [まとめページ](fps_evaluation.md) を参照してください。

点数を $$m$$ として、時間は $$O(m\log^2 m)$$、追加空間は $$O(m\log m)$$ です。
