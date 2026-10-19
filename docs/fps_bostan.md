---
title: Bostan–Mori の NTT 倍長化
documentation_of: //components/fps_bostan.hpp
---

Linux x86_64 / GCC の AVX2 経路で使う内部部品です。通常は公開関数 [Bostan–Mori](https://qandori.github.io/cp-library/fps/bostan_mori.hpp.html) または [線形漸化式](https://qandori.github.io/cp-library/fps/linear_recurrence.hpp.html) を使います。

| 操作 | 内容 | 時間計算量 |
| --- | --- | --- |
| `fps_detail::bostan_detail::inverse_twisted(plan,in,out,powers,n)` | 正規化前の逆 NTT を `in` から `out` へ求め、各要素へ Montgomery 表現の `powers[i]` を掛ける | $$O(n\log n)$$ |

`n` は8以上 $2^{22}$ 以下の2冪とし、`plan` はその変換に必要な根表を持つものとします。`in`・`out`・`powers` は互いに重ならない各 `n` 要素の配列で、32バイト境界に整列させます。各要素は `NTT::U` の生存中のオブジェクトとし、内部ワードの範囲は $[0,2p)$、$p=998244353$ です。入力の変更や配列外の padding は要求しません。呼出側で `NTT::vectorized()` を確認します。

コピーを逆 NTT の最初の8要素の変換へ、係数倍を最後の radix-2 または radix-4 の変換へ融合します。8要素の場合は最初の変換と係数倍をまとめます。Bostan–Mori の倍長化では、分母の最高次項の補正を係数倍の後へ移し、同じ値を得ます。

この部品が利用できない環境では、公開関数は従来のコピー・逆変換・係数倍を使います。公開 API、汎用の係数型への対応、元の変換済み配列の保存は維持します。

**計算量**: $O(n\log n)$ 時間。入力・出力・係数倍・根表を含む空間は $O(n)$、局所変数は $O(1)$ です。根表に付随する3乗根のキャッシュは従来の NTT と同様に必要に応じて更新します。
