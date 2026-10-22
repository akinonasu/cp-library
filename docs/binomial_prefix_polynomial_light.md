---
title: 軽量な二項係数 prefix sum の多項式処理
documentation_of: //components/binomial_prefix_polynomial_light.hpp
---

[軽量な二項係数 prefix sum](binomial_prefix_sum_light.md)の一括処理に使う内部部品です。`light_binomial_prefix_detail::Polynomial<Mint>` が、降階乗基底の多項式を分割統治で評価します。問い合わせを対称性で折り返し、同じ問い合わせをまとめる方式は通常版と同じです。

`Mint=mint` の多項式演算は `light_fps` の積と多項式除算を使います。通常版 FPS とその内部カーネルを読み込みません。他の modint は Karatsuba 法を使います。NTT の上限を超える演算も Karatsuba 法へ戻します。

一般の利用者は `light_combinatorics::binomial_prefix_sum` を使います。内部の `Polynomial` には、前計算済み `Table<Mint>`、折り返した `(m,n)` の配列、元の位置と反転を記録した ID、結果の格納先を渡します。

最大の $$m$$ と異なる問い合わせ数の合計を $$D$$ とします。既定の `mint` で NTT を使える大きさでは $$O(D\log^2 D)$$ 時間、他の modint では $$O(D^{\log_2 3}\log D)$$ 時間です。追加空間は $$O(D\log D)$$ です。
