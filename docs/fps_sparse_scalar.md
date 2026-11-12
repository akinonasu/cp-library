---
title: "軽量 FPS の疎演算"
documentation_of: //components/fps_sparse_scalar.hpp
---

`fps_detail::ScalarSparseSeries` は、非零係数が少ない FPS の逆数・対数・指数・冪を計算する内部部品です。軽量版 FPS から利用します。

`Terms`・`Kind`・`collect`・`get` の引数・返り値・前提条件は [通常版の疎演算](https://qandori.github.io/cp-library/components/fps_sparse.hpp.html) と同じです。係数型は `mint` です。

非零係数数を $$k$$、求める係数数を $$n$$ として、時間 $$O(nk)$$、追加空間 $$O(n+k)$$ です。逆元表の準備時間・空間は別途必要です。
