---
title: "軽量 FPS の基本演算"
documentation_of: //components/fps_light_core.hpp
---

[軽量 FPS](fps_light.md) の型 `LightFormalPowerSeries<T>` と別名 `light_fps`・`LightFPS` を定義する内部部品です。係数型・各演算の引数・返り値・計算量は公開ページを参照してください。

四則演算・逆数・対数・指数・平方根・累乗を含みます。NTT は [共有部品](fps_light_ntt.md) を使います。多点評価・補間・Bostan–Mori などの追加演算と積木は読み込みません。通常は `fps/fps_light.hpp` を include してください。
