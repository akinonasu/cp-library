---
title: 多点評価・補間が共有する積の木
documentation_of: //components/fps_evaluation_common.hpp
---

`fps_detail::evaluation_poly` の内部部品です。通常は [多点評価](multipoint_eval.md) または [補間](interpolate.md) を使います。Linux x86_64 / GCC で定義され、AVX2 が必要です。

`PointTree` は評価点から作る積の木と各節点の変換を保持します。`TreeTransform`・`Arena`、多項式の除算、係数の入出力変換を共有します。評価の降下処理と補間の上昇処理は、それぞれのヘッダーにあります。

評価点数を $$m$$ として、積の木の構築は $$O(m\log^2 m)$$ 時間、$$O(m\log m)$$ 空間です。
