---
title: 行列（軽量実装）
documentation_of: //math/matrix_light.hpp
library_category: math/linear_algebra
redirect_from:
- /math/linear_algebra/matrix_light.hpp.html
- /lightweight/math/linear_algebra/matrix_light.hpp.html
---

`LightMatrix<T>` は [行列](matrix.md) と同じ操作を使える軽量実装です。通常版と同じ共通コードを使い、数値型専用の行列積と Frobenius 分解は読み込みません。通常版の `Matrix<T>` と同じソースで併用できます。

```cpp
#include "lib/math/matrix_light.hpp"
#include "lib/fps/fps_light.hpp"

using M = LightMatrix<light_fps>;
M a{ {light_fps{1}, light_fps{0, 1}},
     {light_fps{0, 1}, light_fps{1}} };
int degree = 8;
auto b = a.pow(1000000000000000000ULL, [degree](M &m) {
    for (int i = 0; i < m.rows(); ++i)
        for (auto &f : m[i])
            if (f.size() > std::size_t(degree)) f.resize(degree);
});
```

この例は $$x^8$$ を法として FPS を要素とする行列を累乗します。`pow(k,normalize)` は単位行列・入力行列と、各積の結果に `normalize` を呼びます。FPS の次数は積のたびに打ち切るため、大きな指数も扱えます。通常版でも `Matrix<fps>` と同じ書き方ができます。

型 `T` に関する条件や、累乗以外の API は [通常版](matrix.md) を参照してください。固定法 modint の行列で速度を重視する場合は通常版を使います。通常版の行列単独も 64 KiB 以内に収まります。

行列積 $$n\times k$$ と $$k\times m$$ は $$O(nkm)$$、正方行列の累乗は $$O(n^3\log k)$$、追加空間は $$O(nk+km+nm)$$ です。要素の演算・コピーを $$O(1)$$ とします。FPS を要素にするときは、その演算の計算量を掛けます。`normalize` の処理時間は別途必要です。
