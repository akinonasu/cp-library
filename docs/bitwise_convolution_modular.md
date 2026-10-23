---
title: ビット演算畳み込みの modint 用共通実装
documentation_of: //components/bitwise_convolution_modular.hpp
---

[AND・OR・XOR 畳み込み](bitwise_convolution.md)が共有する内部処理です。通常は公開関数 `and_convolution`・`or_convolution`・`xor_convolution` を使います。

既定の `mint` と AVX2 に対応する環境で、変換・要素積をまとめて処理します。その他の係数型・環境では公開関数が汎用処理を選びます。

入力長を 2 の冪に切り上げた値を $$N$$ として、時間は $$O(N\log N)$$、追加空間は $$O(N)$$ です。
