---
title: 行列の共通クラス
documentation_of: //components/matrix_core.hpp
---

[通常版の行列](matrix.md) と [軽量版の行列](matrix_light.md) が共有するクラスです。利用するときは、それぞれの公開ヘッダーを include してください。

`Matrix<T,Arithmetic>` は、`Arithmetic` の行列積・体の演算・専用累乗を使います。`Matrix<T>` の既定の演算は、include した行列ヘッダーに応じて通常版または軽量版になります。`LightMatrix<T>` は軽量版を明示する別名です。行列の保存形式と公開操作は共通です。

各操作の計算量は公開側の [行列](matrix.md) と同じです。正方行列の追加空間は $$O(n^2)$$、長方行列の各操作の追加空間は、解空間の出力を除き $$O(nm)$$ です。
