---
title: 乗法的関数の和の内部処理
documentation_of: //components/multiplicative_sum_backend.hpp
---

[乗法的関数の prefix sum](../math/multiplicative_prefix_sum.hpp.html) が、大きい上限での素数和と乗法的関数の和に使う補助部品です。通常は直接使う必要はありません。

平方因子を持たない数に対応する和を作り、強力数上の補正を加えます。素数冪の補正値は共有し、同じ値の再計算を避けます。3 の逆元を使います。

素数和は、完全乗法的な関数の和から素数ごとの Dirichlet 逆変換で復元します。この復元は $$\displaystyle O(\sqrt n\log\log n)$$ 時間です。

作業領域は $$\displaystyle O(\sqrt n)$$ です。
