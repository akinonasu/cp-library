---
title: 多項式の積・打ち切り積の共通処理
documentation_of: //components/polynomial_multiply.hpp
---

`polynomial_detail::Products<F,Limit>` は、積の木と FPS 合成が共有する内部部品です。`F` は `F::multiply` を持つ多項式型、`Limit` はその畳み込みの最大係数数です。

`multiply(a,b)` は通常の積、`multiply_low(a,b,n)` は積の低次 `n` 係数を返します。後者の返り値は長さ `n` で、不足分は 0 です。積が `Limit` を超える場合は係数列を分割して計算します。

分割しない場合の時間は `F::multiply` と同じです。分割する場合は、必要なブロック対ごとの畳み込み時間の合計と、係数の加算時間がかかります。追加空間は出力と各ブロックの作業領域に比例します。
