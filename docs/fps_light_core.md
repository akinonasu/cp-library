---
title: "軽量 FPS の基本演算"
documentation_of: //components/fps_light_core.hpp
---

[軽量 FPS](fps_light.md) の型 `LightFormalPowerSeries<T>` と別名 `light_fps`・`LightFPS` を定義する内部部品です。係数型・各演算の引数・返り値・計算量は公開ページを参照してください。

四則演算・逆数・対数・指数・平方根・累乗を含みます。NTT は [共有部品](fps_light_ntt.md) を使います。多点評価・補間・Bostan–Mori などの追加演算と積木は読み込みません。通常は `fps/fps_light.hpp` を include してください。

密な級数の指数では、Newton 更新で得た逆数の NTT を次の段へ渡します。既知の低次部分の微分との積は短い循環畳み込みで求め、残りの係数を更新します。対数は必要な微分と級数除算を組み合わせます。2〜4 乗では、疎な級数の処理を先に試し、定数項が非零の密な入力には平方と積を使います。

指数の変換結果を再利用する手法は、[NyaanNyaan/library](https://github.com/NyaanNyaan/library/blob/b3981adc80a800b2584980b01821324ea6c77183/fps/ntt-friendly-fps.hpp) と [suisen-cp/cp-library-cpp](https://github.com/suisen-cp/cp-library-cpp/blob/c49e847ae29ed11510ad9cd7713f5ec77ddf20b1/library/polynomial/formal_power_series.hpp) を参考にしています。
