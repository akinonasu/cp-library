---
title: FPS の定数係数畳み込み
documentation_of: //components/fps_convolution_constant.hpp
---

Linux x86_64 / GCC の AVX2 用内部部品です。公開の `fps::multiply` と `convolution` が、両入力の全係数がそれぞれ一定の場合に利用します。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `fps_detail::multiply_detail::constant::fits(a,b)` | 両入力が 4096 項以上で、各列の全係数がそれぞれ同じ剰余か厳密に判定 | 最悪 $O(n+m)$ |
| `fps_detail::multiply_detail::constant::multiply(a,b)` | 確認済みの二つの定数係数列の積を返す | $O(n+m)$ |

`fits` は空入力や短い入力には false を返します。最初に両列の先頭 8 項を比較してから残りを調べるため、先頭で一致しない入力は $O(1)$ で棄却します。遅延剰余を正規化して比較し、末尾まで確認します。

`multiply` の呼び出し側は、AVX2 対応、両 span が生存中の `Mint` 要素を指すこと、各列がそれぞれ定数であること、非空入力、内部 Montgomery 値が $[0,2p)$、出力長 $n+m-1$ が $2^{23}$ 以下であることを保証します。公開 dispatch は `fits` が true のときだけこの関数を呼びます。

各列の係数を $a_0,b_0$ とすると、出力の第 $i$ 係数は $a_0b_0\min(i+1,n,m,n+m-1-i)$ です。正規化した Montgomery 表現の積から等差列を生成し、反対側へ反転して書き、中央の一定区間を埋めます。零の場合も公開サイズを保ち、返すすべての係数を $[0,p)$ に正規化します。

入力を変更せず、alignment や padding を要求しません。同一入力や長短の交換にも対応します。結果は独立した `vector<Mint>` の生存中の要素として構築し、SIMD による入力・出力の受け渡しには `memcpy` を使います。返す配列を除く追加作業領域は $O(1)$、返す配列は $O(n+m)$ です。

採用時の測定と検証は [定数係数畳み込み](https://github.com/qandori/cp-library/blob/main/test/benchmark/fps_convolution_constant_round4.md) にあります。
