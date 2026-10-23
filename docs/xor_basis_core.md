---
title: "XOR 基底の内部演算"
documentation_of: //components/xor_basis_core.hpp
---

`math/linear_algebra/xor_basis.hpp` が使う固定サイズの内部表現です。通常は公開側の `XorBasis` を使います。

| 操作・フィールド | 内容 | 計算量 |
| --- | --- | --- |
| `xor_basis_detail::Core<UInt,Bits>` | 符号なし整数の XOR 基底 | $$O(Bits)$$ |
| `row` / `dimension` | 先頭 `dimension` 個に詰めた基底／次元 | $$O(1)$$ |
| `at` | 各最上位ビットの基底位置に 1 を加えた値。0 は未使用 | $$O(1)$$ |
| `digits` / `mask` | 要素型のビット数／下位 `Bits` bit のマスク | $$O(1)$$ |
| `valid(x)` | `x` が下位 `Bits` bit に収まるか | $$O(1)$$ |
| `pivot(x)` | 最上位の 1 の位置。`x` は非零とする | $$O(1)$$ |
| `insert(x)` | 基底を追加し、格納位置を返す。従属なら -1 | $$O(r)$$ |
| `minimize(x)` / `maximize(x)` | 部分集合の XOR と `x` の XOR の最小値／最大値 | $$O(r)$$ |
| `xor_basis_detail::reduce_rows<UInt,maximum>(x,row,count)` | 消去順の基底に沿って最小化する。`maximum=true` なら最大化 | $$O(\mathrm{count})$$ |

基底は格納順に消去できる形で保持します。行の直接更新は公開ライブラリの内部からだけ行い、`at`・`dimension` も同時に更新します。

構築・初期化・保存領域は $$O(\mathrm{Bits})$$。追加・最小最大は次元を $$r$$ として $$O(r)$$、次元が `Bits` と等しい場合は $$O(1)$$。`valid`・`pivot` は $$O(1)$$ です。
