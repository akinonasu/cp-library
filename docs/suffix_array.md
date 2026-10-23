---
title: Suffix Array Core
documentation_of: //components/suffix_array.hpp
---

Suffix Tree で使う SA-IS による接尾辞配列の構築です。通常は `SuffixTree` を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `suffix_array_detail::bytes(s)` | `string_view` の空でない接尾辞の開始位置を、符号なしバイトの辞書順で返す | $$O(n+\sigma)$$、バイトでは $$O(n)$$ |
| `suffix_array_detail::induced_sort(s,upper)` | `size()` と整数添字を持つ列を、値域 `0`～`upper` として構築する内部処理 | $$O(n+\text{upper})$$ |

`bytes` は空文字列なら空配列を返し、NUL と 128 以上を含む全バイトに対応します。`induced_sort` の要素は非負整数で、`upper` は要素の最大値以上、`INT_MAX-2` 以下にしてください。値域が大きい列は先に座標圧縮が必要です。

少数の要素は直接ソートし、それ以外は型分類・LMS 部分列の名前付け・誘導ソートを行います。LMS 接尾辞の短い prefix で順序が確定する場合や、縮約後の名前が全て異なる場合には、再帰を省きます。確定しない場合は SA-IS を続けます。構築時間・作業空間は $$O(n+\text{upper})$$、バイト列では $$O(n)$$ です。
