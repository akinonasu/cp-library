---
title: 対称群 Fourier 変換
documentation_of: //components/symmetric_group_fft.hpp
---

対称群の係数配列を、各既約表現の行列へ変換します。合成の規約と入力順は [対称群上の畳み込み](symmetric_group_convolution.md) と共通です。通常は畳み込み関数から使いますが、変換後の行列を保存して繰り返し使うこともできます。

```cpp
#include "lib/components/symmetric_group_fft.hpp"
#include <cassert>

SymmetricGroupFFT<> fft(3);
std::vector<mint> a{1, 2, 3, 4, 5, 6};
auto transformed = fft.forward(a);
assert(fft.inverse(transformed) == a);
for (const auto &block : fft.blocks()) {
    assert(block.dimension > 0);
    assert(block.offset + block.dimension * block.dimension <= fft.size());
}
```

| 操作 | 引数・返り値 |
| --- | --- |
| `SymmetricGroupFFT<Mint = mint>(n = 0)` | 次数 `n` の変換を準備。`0 <= n <= 10`、法は `n` より大きい素数 |
| `degree()` | 次数を `int` で返す |
| `size()` | 配列長 `n!` を `int` で返す |
| `blocks()` | `const std::vector<Block>&` を返す。各 `Block` は `partition: vector<int>`・`dimension: int`・`offset: int` |
| `forward(a)` | 辞書順の係数を Fourier 変換し、`std::vector<Mint>` を返す |
| `inverse(a)` | 変換後の配列から辞書順の係数を復元し、`std::vector<Mint>` を返す |

`forward`・`inverse` は `std::span<const Mint>` に変換できる配列を受け取ります。入力は変更しません。長さが `size()` と異なる場合は `std::invalid_argument` を送出します。構築時の次数が範囲外、または法が `n` 以下の場合も `std::invalid_argument` です。法の素数判定は行いません。`Mint` に必要な演算と動的 modint の条件は [畳み込み](symmetric_group_convolution.md) と同じです。

`partition` は `n` の分割で、降順の整数列です。各行列は行優先で格納し、`a[block.offset + i * block.dimension + j]` が行 `i`・列 `j` の要素です。行・列はともに `0 <= i, j < dimension` です。ブロックの要素数の合計は `size()` です。`blocks()` の参照はオブジェクトが破棄・代入されるまで有効です。

`forward(a)` を $$A_\lambda$$ とすると、同じ変換で得た $$B_\lambda$$ との行列積 $$A_\lambda B_\lambda$$ をブロックごとに並べて `inverse` に渡すと、$$q[r[i]]$$ の規約での畳み込みになります。任意の長さ `n!` の配列について、`forward(inverse(a))`・`inverse(forward(a))` はともに元の配列に戻ります。`n = 0, 1` では変換は恒等写像です。

変換の準備では Young 図形と標準盤から計算手順を構成します。係数への計算は部分群に沿った Horner 型の手順で行い、逆変換にはその転置を用います。平方根や乱数は使いません。

各変換は $$O(n^2n!)$$ 時間・$$O(n!)$$ 追加作業空間です。計算手順の保持空間は $$O(nn!)$$、構築時間は辞書による手順構築を含む上界で $$O(n^2n!\log(n!))$$ です。体の四則演算を $$O(1)$$ とし、逆元の計算は構築時に $$O(n)$$ 回、逆変換時に 1 回行います。`n = 0, 1` は $$O(1)$$ 時間・空間です。
