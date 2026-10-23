---
title: "FPS の合成・射影の汎用実装"
documentation_of: //components/fps_composition_generic.hpp
---

合成・power projection・合成逆元を FPS の型で共通化した内部実装です。軽量 FPS と、高速 FPS の専用 SIMD 経路を使わない場合に共有します。通常は各演算の公開ヘッダーを include してください。

二変数 Bostan–Mori の各段を一次元畳み込みへ埋め込み、合成は射影の転置として計算します。積・逆数・対数・指数などの演算は渡された FPS の型へ委譲し、係数の表現や NTT の実装を固定しません。

対象の係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、各段の分母を含む追加空間は $$O(n\log n)$$ です。
