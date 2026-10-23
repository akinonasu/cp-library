---
title: 区間アフィン変換・区間和の共通実装
documentation_of: //components/range_affine_sum_simd.hpp
---

[区間アフィン変換・区間和](range_affine_range_sum.md)が使う内部処理です。通常は公開クラス `RangeAffineRangeSum` を使います。

静的な奇数法が $$2^{30}$$ 未満で AVX2 を使える場合に、多分岐の木の区間和と遅延作用をまとめて更新します。対応する命令の有無は公開クラスが判定します。

列長を $$n$$ として、構築時間・使用空間は $$O(n)$$、更新・区間和は $$O(\log n)$$ 時間です。
