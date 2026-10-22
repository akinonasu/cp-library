---
title: "軽量 FPS の積木"
documentation_of: //components/fps_light_product_tree.hpp
---

```cpp
#include "lib/components/fps_light_product_tree.hpp"

std::vector<mint> xs{0, 1, 2};
fps_light_detail::ProductTree<mint> tree(xs);
auto values = tree.evaluate(light_fps{1, 2, 3}); // {1,6,17}
```

[軽量 FPS の多点評価・補間](https://qandori.github.io/cp-library/fps/fps_light_algorithms.hpp.html) が共有する内部部品です。各葉に一次式を置き、上の節点ほど積を保持します。評価は多項式の剰余を子へ渡して行います。通常は公開関数 `light_fps_algorithms::multipoint_eval`・`interpolate` を使ってください。

| 要素 | 内容 |
| --- | --- |
| `ProductTree<T>(xs)` | 評価点から積木を構築。点の重複・空の点列を許す |
| `tree.evaluate(f)` | `xs` の順の評価値を `std::vector<T>` で返す |
| `tree.count` / `tree.size` | 点数／点数以上の最小の 2 冪（空なら 1） |
| `tree.product` | 1 始まりの節点配列。添字 0 は未使用 |

`product[1]` は $$\displaystyle \prod_{i=0}^{m-1}(x-x_i)$$ です。空の点列では定数多項式 1 です。`product[size+i]` は有効な点では $$x-x_i$$、末尾の埋め草では定数 1 です。構築後は `product`・`size`・`count` を変更しないでください。

法と係数型は [軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) と同じ条件です。点数は $$2^{23}$$ 未満で、構築・評価の途中の積も含めて必要な NTT 長が $$2^{23}$$ 以下である必要があります。

点数を $$m$$、評価する多項式の係数数を $$n$$ として、構築は $$O(m\log^2 m)$$ 時間・$$O(m\log m)$$ 空間です。評価は $$O((n+m)\log^2(n+m))$$ 時間・$$O(n+m)$$ 追加空間で、構築済みの積木の空間は別に必要です。
