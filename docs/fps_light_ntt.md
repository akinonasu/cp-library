---
title: "軽量 FPS の NTT"
documentation_of: //components/fps_light_ntt.hpp
---

通常は [軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) を利用します。法 998244353 の共有 `mint` と小さい NTT を使う内部部品です。既存の大型 NTT・演算別カーネルには依存しません。

| 関数 | 内容・条件 |
| --- | --- |
| `fps_light_detail::multiply(a,b)` | `std::vector<mint>` 同士の畳み込み。入力が空なら空配列 |
| `fps_light_detail::transform(a,inverse = false)` | 順変換／逆変換。長さは正の 2 冪で $$2^{23}$$ 以下 |

順変換は DIF、逆変換は DIT で、変換領域の係数は bit-reversed 順です。順変換後の各点積と逆変換を組み合わせる用途で使います。根の表は必要な長さまで共有して拡張します。x86 の GCC・Clang では AVX2 の対応を実行時に確認し、対応する CPU では大きい段のバタフライを SIMD で処理します。それ以外はスカラー経路です。

積の結果の長さは両入力が非空なら `a.size()+b.size()-1` です。各入力と結果の長さを $$2^{23}$$ 以下にしてください。小さい入力は愚直積、同じ配列を二度渡した平方では変換を一度だけ行います。

積の結果長または変換長を $$n$$ として、時間は $$O(n\log(n+1))$$、追加空間は $$O(n)$$ です。根の表は、これまでに必要になった最大変換長を $$L$$ として $$O(L)$$ 空間を保持します。
