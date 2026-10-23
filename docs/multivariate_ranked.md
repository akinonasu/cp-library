---
title: 小さい次元の多変数畳み込み
documentation_of: //components/multivariate_ranked.hpp
---

[多変数の切り捨て畳み込み](multivariate_convolution.md)が使う内部処理です。各変数の次元が 2 または 3 の場合に、次数ごとの変換と積をまとめて計算します。通常は公開関数 `multivariate_convolution(a,b,dimensions)` を使います。

1 より大きい次元数を $$D$$、入力長を $$N$$ として、時間は $$O(D^2N)$$、追加空間は $$O(DN)$$ です。
