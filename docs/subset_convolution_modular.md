---
title: Subset Convolution Modular Core
documentation_of: //components/subset_convolution_modular.hpp
---

[Subset 畳み込み](subset_convolution.md)の modint 用の内部処理です。通常は `subset_convolution(a,b)` を使います。

静的な奇数法が $$2^{30}$$ 未満で、対応CPUを使える場合に専用の変換・積を行います。それ以外の型・環境では公開関数が汎用の環演算を選びます。

時間は $$O(N\log^2 N)$$、追加空間は $$O(N\log N)$$ です。$$N$$ は長い方の入力長を2の冪に切り上げた値です。
