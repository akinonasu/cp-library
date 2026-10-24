---
title: 多項式の固定長シフトの共通処理
documentation_of: //components/polynomial_shift.hpp
---

`polynomial_detail::shift_fixed<Left>(a, distance)` は、FPS の通常版と軽量版が共有する内部部品です。`a` の長さを保ち、`Left` が `true` なら高次側、`false` なら低次側へ係数を移動します。空いた係数は 0、範囲外の係数は破棄します。返り値は `a` への参照です。

`distance` は非負の `int` です。長さ以上なら全係数を 0 にし、空列と移動量 0 はそのまま返します。`F` は要素型 `value_type` と、書き換え可能なランダムアクセスイテレータを持つ型を指定します。

長さを $$n$$ として、時間 $$O(n)$$、追加空間 $$O(1)$$ です。係数の移動と 0 の代入の計算量を定数とします。
