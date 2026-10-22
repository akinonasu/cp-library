---
title: "軽量 FPS の合成・射影の共通実装"
documentation_of: //components/fps_composition_light.hpp
---

[軽量 FPS の合成](https://qandori.github.io/cp-library/fps/compose_light.hpp.html)、[power projection](https://qandori.github.io/cp-library/fps/power_projection_light.hpp.html)、[合成逆元](https://qandori.github.io/cp-library/fps/compositional_inverse_light.hpp.html)で共有する実装です。通常は各公開ヘッダーを include してください。

二変数 Bostan–Mori の各段を通常の一次元畳み込みへ埋め込みます。分母の積は偶数次・奇数次に分解して変換長を抑え、合成は射影の転置として計算します。必要な畳み込みと級数演算はすべて軽量 FPS に委譲し、高速実装の FPS や専用 NTT は読み込みません。

通常の ASCII の C++ として直接編集できます。[実装・検証資料](https://github.com/qandori/cp-library/blob/main/test/benchmark/fps_light_composition.md)を参照してください。

対象の係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、保持する各段の分母を含む追加空間は $$O(n\log n)$$ です。
