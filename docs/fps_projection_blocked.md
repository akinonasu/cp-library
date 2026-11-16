---
title: "FPS の power projection の専用変換"
documentation_of: //components/fps_projection_blocked.hpp
library_category: components/fps
---

[power projection](https://qandori.github.io/cp-library/fps/power_projection.hpp.html)と[合成逆元](https://qandori.github.io/cp-library/fps/compositional_inverse.hpp.html)が使う Linux x86_64 / GCC 向けの内部実装です。呼び出し側で AVX2 対応を確認して使います。[合成・power projection の共通変換](fps_composition_common.md)を読み込み、合成専用の処理は読み込みません。

power projection は、各冪 $$f(x)^0,f(x)^1,\ldots$$ から、指定した次数の係数、または係数の重み付き和を取り出して並べる操作です。

`projection_log`・`projection_scratch`・`power_projection`・`weighted_projection`・`native_projection`・`native_weighted_projection` を提供します。整数配列を直接渡す内部 API の条件は[合成・power projection の内部 API](fps_composition.md)を参照してください。両方の演算が必要なら `components/fps_composition.hpp` を使えます。

対象の係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、追加空間は $$O(n)$$ です。
