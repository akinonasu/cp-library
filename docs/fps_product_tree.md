---
title: "多点評価・補間の内部木"
documentation_of: //components/fps_product_tree.hpp
---

`fps_detail::ProductTree<T>` は [多点評価](https://qandori.github.io/cp-library/fps/multipoint_eval.hpp.html)・[補間](https://qandori.github.io/cp-library/fps/interpolate.hpp.html) が共有する内部処理です。通常は各関数のヘッダーを include してください。内部で使う区間積と NTT のキャッシュを構築します。

| 操作 | 内容・返り値 |
| --- | --- |
| `build(xs,size,block,reciprocal,frequency = nullptr)` | 各点の一次式を積にまとめる木を返す。`size` を必要な 2 冪まで増やす。`reciprocal` は係数順、`frequency` は変換結果の保存先 |
| `evaluate(points,tree,frequency,size,block,root,consume = false)` | 木と変換キャッシュを使って評価値の配列を返す。`consume` が真ならキャッシュを再利用・消費 |

`size * block` を $$n$$ として、時間 $$O(n\log^2 n)$$、木・キャッシュを含む追加空間は $$O(n\log n)$$ です。
