---
title: 乗法的関数の和の内部処理
documentation_of: //components/multiplicative_sum_backend.hpp
---

[乗法的関数の prefix sum](../math/number_theory/multiplicative_prefix_sum.hpp.html) が、大きい上限での素数和と乗法的関数の和に使う補助部品です。通常は直接使う必要はありません。

平方因子を持たない数に対応する和を作り、強力数上の補正を加えます。3 の逆元を使います。

作業領域は $$\displaystyle O(\sqrt n)$$ です。
