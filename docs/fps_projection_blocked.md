---
title: "FPS の射影の専用変換"
documentation_of: //components/fps_projection_blocked.hpp
---

[power projection](https://qandori.github.io/cp-library/fps/power_projection.hpp.html)と[合成逆元](https://qandori.github.io/cp-library/fps/compositional_inverse.hpp.html)が使う Linux x86_64 / GCC 向けの内部実装です。呼び出し側で AVX2 対応を確認して使います。[合成・射影の共通変換](fps_composition_common.md)を読み込み、合成専用の処理は読み込みません。

二変数 Bostan–Mori の分子・分母を段階的に更新し、係数射影と重み付き射影を計算します。`projection_log`・`projection_scratch`・`power_projection`・`weighted_projection`・`native_projection`・`native_weighted_projection` を提供します。raw API の条件は[合成・射影の内部 API](fps_composition.md)を参照してください。両方の演算が必要なら `components/fps_composition.hpp` を使えます。

対象の係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、追加空間は $$O(n)$$ です。
