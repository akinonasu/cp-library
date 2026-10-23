---
title: "疎 FPS の次数圧縮・前処理"
documentation_of: //components/fps_sparse_core.hpp
---

`fps_detail::SparseBase<Engine>` は、疎 FPS の項収集・次数の最大公約数による圧縮・小さな整数冪の計算を共有する内部部品です。[通常版の疎演算](https://qandori.github.io/cp-library/components/fps_sparse.hpp.html) と [軽量版の疎演算](https://qandori.github.io/cp-library/components/fps_sparse_scalar.hpp.html) から使います。

`Terms`・`Kind`・`collect`・`get` の引数・返り値・前提条件は通常版と同じです。`Engine` は `solve<kind>`・`geometric<exponential>`・`scale` を提供します。`get` は次数を圧縮して演算を委譲し、結果を元の次数へ戻します。SIMD 演算には依存しません。

非零係数数を $$k$$、求める係数数を $$n$$ として、演算を含む時間は $$O(nk)$$、追加空間は $$O(n+k)$$ です。
