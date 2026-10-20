---
title: "Power Projection"
documentation_of: //fps/power_projection.hpp
---

```cpp
#include "lib/fps/power_projection.hpp"

fps f{0, 1, 1}, w{1, 2, 3};
auto a = power_projection(f, w, 4); // {1, 5, 3, 0}
auto b = power_projection(f, 2, 4); // {0, 1, 1, 0}
```

法は 998244353 です。`f`・`w` は変更せず、返り値は `FormalPowerSeries<T>` です。`f` の定数項は任意です。

返り値の添字 `i` は `f` を何乗するかを表します。係数版の第 2 引数を 2 にすると、`f^0,f^1,f^2,...` のそれぞれから $$x^2$$ の係数を取り出して並べます。重み付き版では、それぞれの冪の係数列と `w` の内積を並べます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `power_projection(f,w,deg = -1)` | $$\displaystyle a_i=\sum_{j=0}^{n-1}w_j[x^j]f(x)^i$$ を $$0\le i<\mathrm{deg}$$ について返す。$$n=\texttt{w.size()}$$。省略時の長さは `w.size()` | f[0]=0: $$O(n\log^2 n+m)$$、その他: $$O(n\log^2 n+m\log m)$$ |
| `power_projection(f,n,deg = -1)` | $$a_i=[x^n]f(x)^i$$ を返す。$$n\ge0$$。省略時の長さは `n + 1` | f[0]=0: $$O(n\log^2 n+m)$$、その他: $$O(n\log^2 n+m\log m)$$ |

`deg` は返す要素数です。0 なら空列、空の重みなら零列を返します。空の `f` は零多項式として扱い、$$f(x)^0=1$$ とします。重み付き版の `w.size()`、係数版の `n + 1` は $$2^{21}$$ 以下、`deg` は $$2^{22}$$ 以下とします。

$$[x^n]g(x)f(x)^i$$ が必要なら、長さ `n + 1` の重みを $$w_j=g_{n-j}$$ として渡します。`f` と重みの長さを揃える必要はありません。不要な高次係数は参照しません。

$$f_0=0$$ のときは、[FPS の合成](https://qandori.github.io/cp-library/fps/compose.hpp.html)の転置にあたります。$$a_i$$ と外側の係数 $$h_i$$ の内積は、$$h(f(x))\bmod x^n$$ と重みの内積に一致します。

定数項以外の非零係数の次数に公約数がある場合は、`f` と対応する重みをその間隔で圧縮して計算します。

Linux x86_64 / GCC と AVX2 では、零となる列の変換を省く専用の射影処理を使います。係数版に加え、一般の重み付き版にも対応します。

重み数を $$n$$、出力数を $$m$$ として、時間は $$f_0=0$$ なら $$O(n\log^2 n+m)$$、一般には $$O(n\log^2 n+m\log m)$$、追加空間は $$O(n+m)$$ です。空入力・定数・定数項を除いて単項式の場合には専用の処理を使います。
