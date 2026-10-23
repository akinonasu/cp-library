---
title: 対称群上の畳み込み
documentation_of: //convolution/symmetric_group_convolution.hpp
---

長さ $$n$$ の全順列を添字とする配列の畳み込みを求めます。通常の加算や XOR の代わりに、順列の合成を用います。

$$c_p=\displaystyle\sum_{q\circ r=p}a_qb_r,\qquad(q\circ r)[i]=q[r[i]].$$

順列は `0, ..., n-1` を並べたものとし、配列は **辞書順**に並べます。長さは $$n!$$ です。合成は交換可能ではなく、一般に `convolution(a, b)` と `convolution(b, a)` は異なります。

```cpp
#include "lib/convolution/symmetric_group_convolution.hpp"
#include <cassert>

std::vector<mint> a{1, 2};
std::vector<mint> b{3, 4};
auto c = symmetric_group_convolution(a, b);
assert((c == std::vector<mint>{11, 10}));
```

長さ 2 では順列は `[0, 1]` と `[1, 0]` の順です。前者は単位元、後者は自分自身との合成で単位元になります。一般の順列と添字の相互変換には [順列の辞書順順位・復元](permutation_rank.md) を使えます。

| 操作 | 引数・返り値 |
| --- | --- |
| `symmetric_group_convolution(a, b)` | 同じ長さの `const std::vector<Mint>&` 2 本から `std::vector<Mint>` を返す。長さから `n` を決定 |
| `symmetric_group_convolution(fft, a, b)` | 構築済みの `const SymmetricGroupFFT<Mint>&` を使い、同じ畳み込みを返す |

入力は変更しません。型 `Mint` は両配列で同じ modint 型とし、法は **`n` より大きい素数**を使います。`mint`・`atcoder::modint998244353`・`atcoder::modint1000000007`・動的 modint に対応します。整数からの構築・四則演算・単項マイナス・`mod()` を備える型を指定してください。動的 modint の法は `fft` の構築から使用終了まで変更しません。

$$0\le n\le10$$ に対応します。空配列、長さが一致しない配列、長さが対応する階乗でない配列には `std::invalid_argument` を送出します。長さ 1 はスカラーの積を返します。これは空順列の群でも長さ 1 の順列の群でも同じ結果です。構築済み `fft` を渡す場合は、その `size()` と配列長を一致させます。

同じ次数で繰り返し計算するときは、変換の準備を共有できます。

```cpp
#include "lib/convolution/symmetric_group_convolution.hpp"
#include <cassert>

SymmetricGroupFFT<> fft(3);
std::vector<mint> a{1, 2, 3, 4, 5, 6};
std::vector<mint> unit(6);
unit[0] = 1;
assert(symmetric_group_convolution(fft, a, unit) == a);
assert(symmetric_group_convolution(fft, unit, a) == a);
```

Maslen の対称群 Fourier 変換を使います。部分群への分割を Horner 型にまとめ、Young 図形の経路で中間計算を共有します。有限体上では平方根を必要としない有理的な Young の半正規表現を使います。Fourier 変換した各行列を順に掛け、転置アルゴリズムによる逆変換で係数に戻します。変換自体の API は [対称群 Fourier 変換](symmetric_group_fft.md) を参照してください。

参考: [Library Checker issue #1432](https://github.com/yosupo06/library-checker-problems/issues/1432)、David K. Maslen, [The efficient computation of Fourier transforms on the symmetric group](https://www.ams.org/journals/mcom/1998-67-223/S0025-5718-98-00964-8/)、同手法の [FPSAC 1997 発表版](https://fpsac-archive.github.io/FPSAC97/articles.html)。issue は問題案のため、公式ジャッジ用テストは追加していません。

$$n$$ の分割 $$\lambda$$ に対応する表現の次元を $$d_\lambda$$ とすると、$$\displaystyle\sum_\lambda d_\lambda^2=n!$$ です。畳み込みは前計算後 $$O\left(n^2n!+\displaystyle\sum_\lambda d_\lambda^3\right)$$ 時間です。前計算・保持空間は [Fourier 変換](symmetric_group_fft.md) と共通で、計算時の追加作業空間は $$O(n!)$$ です。体の演算を $$O(1)$$ とします。
