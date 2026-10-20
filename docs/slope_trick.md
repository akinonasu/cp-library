---
title: "Slope Trick"
documentation_of: //data_structure/slope_trick.hpp
---

```cpp
#include "lib/data_structure/slope_trick.hpp"

SlopeTrick<> f;
f.add_abs(3);
f.add_x_minus_a(1);
f.shift(-1, 2);
auto value = f.min();
auto [left, right] = f.argmin();
auto y = f.eval(5);
```

初期状態は $$f(x)=0$$ です。座標型・値の型は `SlopeTrick<T, Sum>` で指定し、既定はともに `std::int64_t` です。座標と遅延値の計算は `T`、関数値の計算は `Sum` に収めてください。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SlopeTrick()` | $$f(x)=0$$ で初期化 | $$O(1)$$ |
| `add_all(c)` | $$f(x)+c$$ | $$O(1)$$ |
| `add_a_minus_x(a)` | $$f(x)+\max(a-x,0)$$ | 償却 $$O(\log n)$$ |
| `add_x_minus_a(a)` | $$f(x)+\max(x-a,0)$$ | 償却 $$O(\log n)$$ |
| `add_abs(a)` | $$f(x)+\lvert x-a\rvert$$ | 償却 $$O(\log n)$$ |
| `shift(a)` | $$f(x-a)$$ | $$O(1)$$ |
| `shift(a,b)` | $$\displaystyle \min_{x-b\le y\le x-a}f(y)$$、$$a\le b$$ | $$O(1)$$ |
| `clear_right()` | $$\displaystyle \min_{y\le x}f(y)$$ | $$O(1)$$ |
| `clear_left()` | $$\displaystyle \min_{y\ge x}f(y)$$ | $$O(1)$$ |
| `min()` | 最小値 | $$O(1)$$ |
| `argmin()` | 最小値を取る閉区間の両端。無限側は `nullopt` | $$O(1)$$ |
| `eval(x)` | $$f(x)$$。関数は変更しない | $$O(n)$$ |
| `merge(g)` / `merge(std::move(g))` | $$f(x)+g(x)$$。`g` は初期状態に戻る | $$O(\min(n,m)\log(n+m))$$、左辺値のコピーは別途 $$O(m)$$ |
| `size()` / `empty()` | 傾きの変化点数／変化点がないか | $$O(1)$$ |
| `reserve(n)` / `clear()` | 左右にそれぞれ `n` 点分を予約／容量を残して初期状態に戻す | reserve: $$O(n)$$、clear: $$O(1)$$ |
| `swap(g)` | 関数と確保領域を交換 | $$O(1)$$ |

変化点数を $$n$$ として、折れ線の追加は償却 $$O(\log n)$$、`eval` は $$O(n)$$、その他の操作は $$O(1)$$ です。変化点数 $$m$$ の関数との `merge` は $$O(\min(n,m)\log(n+m))$$、空間は $$O(n)$$ です。コピー・予約は $$O(n)$$、演算と要素のコピーは $$O(1)$$ とします。
