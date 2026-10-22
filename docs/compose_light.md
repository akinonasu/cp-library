---
title: "軽量 FPS の合成"
documentation_of: //fps/compose_light.hpp
---

```cpp
#include "lib/fps/compose_light.hpp"

light_fps f{1, 2, 3}, g{0, 1, 1};
auto h = light_fps_algorithms::compose(f, g, 4); // {1, 2, 5, 6}
```

`compose(f,g,deg = -1)` は $$f(g(x))\bmod x^{\mathrm{deg}}$$ の係数列を返します。`deg` は返す係数数で、省略時は `f.size()`、0 なら空列です。`g` は空列または定数項が 0 である必要があります。入力は変更しません。

[軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) と同じ `LightFormalPowerSeries<T>` を使います。高速実装の FPS やその NTT には依存しません。二変数 Bostan–Mori による power projection の転置を、軽量 FPS の通常の畳み込みで計算します。コードを短縮するための特殊文字マクロは使いません。

`deg` は $$2^{21}$$ 以下とします。法や係数型の条件は軽量 FPS と同じです。

出力係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、追加空間は $$O(n\log n)$$ です。
