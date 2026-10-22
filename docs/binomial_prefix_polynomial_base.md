---
title: "二項係数 prefix sum の共通多項式処理"
documentation_of: //components/binomial_prefix_polynomial_base.hpp
---

二項係数 prefix sum の通常版・軽量版で共有する内部処理です。降階乗基底の構築、問い合わせの集約、分割統治による多点評価をまとめます。FPS や特定の modint には依存しません。

`binomial_prefix_detail::PolynomialEvaluator<Mint,Arithmetic,Table>` に、係数型、多項式演算、階乗などの前計算表を渡します。`PolynomialScalarArithmetic<Mint>` は一般の係数型で使う演算、`PolynomialSeriesArithmetic<Mint,Series>` は指定した FPS の乗算・剰余を使う演算です。通常は [二項係数の prefix sum](binomial_prefix_sum.md) または [軽量版](binomial_prefix_sum_light.md) を利用します。

最大の $$m$$ と異なる問い合わせ数の合計を $$D$$ とします。問い合わせのソートを除き、NTT を利用する場合は $$O(D\log^2 D)$$ 時間、Karatsuba 法では $$O(D^{\log_2 3}\log D)$$ 時間、追加空間は $$O(D\log D)$$ です。
