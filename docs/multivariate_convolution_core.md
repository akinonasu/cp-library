---
title: 多変数畳み込みの共通実装
documentation_of: //components/multivariate_convolution_core.hpp
---

[多変数の切り捨て畳み込み](multivariate_convolution.md)と[軽量版](multivariate_convolution_light.md)が共有する mod 998244353 用の内部処理です。通常は公開関数 `multivariate_convolution(a,b,dimensions)` を使います。

変数の次元を分割し、次数による重み付け・変換・点ごとの積を組み合わせます。対応する x86 CPU では要素積などを AVX2 で処理し、それ以外では通常の整数演算を使います。

1 より大きい次元数を $$D$$、入力長を $$N$$ として、時間は $$O(DN\log N+D^2N)$$、追加空間は $$O(ND)$$ です。
