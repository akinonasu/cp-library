---
title: "FPS の合成の専用変換"
documentation_of: //components/fps_compose_blocked.hpp
---

[合成](https://qandori.github.io/cp-library/fps/compose.hpp.html)が使う Linux x86_64 / GCC 向けの内部実装です。呼び出し側で AVX2 対応を確認して使います。[合成・射影の共通変換](fps_composition_common.md)を読み込み、射影専用の処理は読み込みません。

Kinoshita–Li 法の各段とその転置を使い、外側の係数に線形な合成を計算します。`compose_log`・`compose_scratch`・`compose`・`native_compose` を提供します。raw API の引数、係数表現、作業領域の条件は[合成・射影の内部 API](fps_composition.md)を参照してください。両方の演算が必要なら `components/fps_composition.hpp` を使えます。

対象の係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、各段の情報を含む追加空間は $$O(n\log n)$$ です。
