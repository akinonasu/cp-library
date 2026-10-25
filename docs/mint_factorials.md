---
title: modint998244353 の階乗・逆階乗生成
documentation_of: //components/mint_factorials.hpp
---

FPS と二項係数の前計算で共有する内部処理です。NTT には依存せず、共通の剰余演算を使います。通常は [階乗・逆階乗と二項係数](https://qandori.github.io/cp-library/math/comb.hpp.html) を利用します。

`mint_detail::factorials(fact,invfact,n)` は `fact[i] = i!`、`invfact[i] = 1/i!` を先頭 `n` 要素へ書き込みます。`mint*` 版では $$0\le n\le998244353$$、内部 Montgomery ワードのポインタ版では $$0\le n<998244353$$ とし、それぞれ `n` 要素を持つ重ならない出力領域を渡します。`n = 0` では書き込みません。

256 要素以上の AVX2 対応 x86 では SIMD 版を、それ以外ではスカラー版を使います。`factorials8`・`factorial_chain`・`factorial_transpose` は x86 向けの内部関数です。

生成は $$O(n)$$ 時間・$$O(1)$$ 追加空間と逆元計算 1 回です。
