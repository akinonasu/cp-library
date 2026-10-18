---
title: "形式的冪級数"
documentation_of: //fps/fps.hpp
---

```cpp
#include "lib/fps/fps.hpp"

fps f{1, 2, 3}, g{4, 5};
fps product = f * g;
fps inverse = f.inv(10);
```

mod は 998244353、係数は定数項から順に格納します。`fps` は `std::vector<mint>` と同様に扱えます。`deg` は返す係数の個数で、省略時は元のサイズです。

`fps{1,2,3}` は $$1+2x+3x^2$$ を表します。係数 `f[i]` は `mint` なので、整数として取り出すには `f[i].val()` を使います。`f.inv(10)` などの 10 は最高次数ではなく返す長さで、$$x^0$$ から $$x^9$$ までを計算します。

`f.pow(k)`・微分・積分・シフトは元の長さで打ち切ります。例えば `fps{1,1}.pow(3)` は `{1,3}` です。4 係数が必要なら `f.resize(4)` としてから `f.pow(3)` を呼びます。一方、通常の積 `f*g` は両入力が非空なら長さ `f.size()+g.size()-1` を返します。

`f/g` は指定長までの級数の除算です。多項式の割り算の商と余りを求める場合は `auto [q,r] = f.divmod(g)` とします。これらの戻り値は新しい配列で、元の `f` を更新する演算には `*=` などの複合代入を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps()` / `fps(n)` / `fps{a0,a1,...}` / `fps(vector<mint>)` | 空列／長さ指定／係数列から構築 | 空: $$O(1)$$、長さ指定: $$O(n)$$ |
| `f = a` / `f = {a0,a1,...}` | 配列／初期値列を代入 | $$O(n)$$ |
| `f[i]` / `f.size()` / `f.resize(n)` / `f.empty()` | `std::vector<mint>` と同じ要素取得・サイズ操作。イテレータや `push_back` なども利用可 | 参照: $$O(1)$$、resize: $$O(n)$$ |
| `fps::multiply(f,g)` / `f * g` | 多項式の積 | $$O(n\log n)$$ |
| `-f` / `f + g` / `f - g` | 符号反転／加算／減算 | $$O(n)$$ |
| `f + c` / `f - c` / `f * c` / `f / c` | 定数項への加減算／全係数の定数倍・除算。除数は非零 | $$O(n)$$ |
| `f += g` / `f -= g` / `f *= g` / `f /= g` / `f %= g` | 演算結果を代入。スカラーとの複合代入も利用可 | 加減算: $$O(n)$$、乗除算・剰余: $$O(n\log n)$$、定数項の加減算: $$O(1)$$ |
| `f << d` / `f >> d` / `f <<= d` / `f >>= d` | 係数列をずらし、元のサイズを保つ。`d >= 0` | $$O(n)$$ |
| `f.pre(n)` | 先頭から高々 `n` 個の係数を返す | $$O(n)$$ |
| `f / g` / `f.div_series(g,deg = -1)` | 級数の除算。$$g_0 \ne 0$$ | $$O(n\log n)$$ |
| `f.divmod(g)` / `f % g` | 多項式の商と余り／余り。$$g \ne 0$$ | $$O(n\log n)$$ |
| `f.inv(deg = -1)` | 逆数。$$f_0 \ne 0$$ | $$O(n\log n)$$ |
| `f.log(deg = -1)` / `f.exp(deg = -1)` | 対数／指数。定数項はそれぞれ 1／0 | $$O(n\log n)$$ |
| `f.sqrt(deg = -1)` | 平方根。存在しなければ空配列 | $$O(n\log n)$$ |
| `f.pow(k)` | 非負整数乗。元のサイズで打ち切る | $$O(n\log n+\log k)$$ |
| `f.diff()` / `f.integral()` | 微分／定数項 0 の積分。元のサイズを保つ | $$O(n)$$ |
| `f.eval(x)` | 一点評価 | $$O(n)$$ |
| `f.onemul(d,c)` / `f.onediv(d,c)` | $$1+cx^d$$ を掛ける／割る。`d >= 0`、自身のサイズを保つ。除算で `d = 0` なら `c != -1` | $$O(n)$$ |

[Bostan–Mori](https://qandori.github.io/cp-library/fps/bostan_mori.hpp.html)、[線形漸化式](https://qandori.github.io/cp-library/fps/linear_recurrence.hpp.html)、[合成](https://qandori.github.io/cp-library/fps/compose.hpp.html)、[合成逆元](https://qandori.github.io/cp-library/fps/compositional_inverse.hpp.html)、[多点評価](https://qandori.github.io/cp-library/fps/multipoint_eval.hpp.html)、[補間](https://qandori.github.io/cp-library/fps/interpolate.hpp.html)、[Taylor shift](https://qandori.github.io/cp-library/fps/taylor_shift.hpp.html) は専用ヘッダーの通常の関数として使います。本ヘッダーには含まれません。

Linux x86_64 / GCC では AVX2 対応を実行時に確認し、積・逆数・対数・指数・冪・平方根・除算に専用のブロック変換を使います。部分木内で変換と積を融合し、Newton 更新や隣接区間の中間結果を再利用します。係数を内部の Montgomery 表現に保ち、結果の二重初期化や不要な入力コピーを省きます。疎な入力や小さな次数には既存の専用経路を使います。他の環境でも同じ API で動作します。

入力・出力の最大係数数を $$n$$ とします。

| 操作 | 時間計算量 |
| --- | --- |
| 加減算・スカラー演算・`diff`・`integral`・`onemul`・`onediv`・一点評価 | $$O(n)$$ |
| 乗算・級数除算・多項式除算・`inv`・`log`・`exp`・`sqrt` | $$O(n \log n)$$ |
| `pow(k)` | $$O(n \log n + \log k)$$ |

追加空間は $$O(n)$$ です。
