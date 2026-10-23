---
title: "軽量な形式的冪級数"
documentation_of: //fps/fps_light.hpp
---

```cpp
#include "lib/fps/fps_light.hpp"

light_fps f{1, 2, 3}, g{4, 5};
auto product = f * g; // {4,13,22,15}
auto inverse = f.inv(10);
auto logarithm = f.log(10);
```

Codeforces などの小さいソースサイズ上限向けの FPS です。法は 998244353、係数は定数項から順に格納します。`light_fps` と `LightFPS` は `LightFormalPowerSeries<mint>` の別名で、`std::vector<mint>` と同様の要素アクセス・イテレータ・サイズ変更を使えます。

[高速実装の FPS](https://qandori.github.io/cp-library/fps/fps.hpp.html) と別の型で、同じソース内に両方を include できます。軽量版は小さい NTT と Newton 法を共有し、高速実装の大きな演算別カーネルを読み込みません。提出用コードはヘッダーを結合して作り、利用者のコードやほかのライブラリを含めた総バイト数を確認してください。

`LightFormalPowerSeries<T>` は `mod()` が定数式となる法 998244353 の modint 型を指定できます。`T` は整数からの構築、`mod()`・`raw()`・`val()`・`inv()`・`pow()`、四則演算と等値比較に対応する必要があります。既定型以外は積・逆数・対数・指数・平方根などの演算ごとに共有する `mint` へ変換します。

`deg` は返す係数数で、省略時は元の長さです。逆数の例は $$x^0$$ から $$x^9$$ までを返します。通常の積は入力が非空なら長さ `f.size()+g.size()-1`、`diff()`・`integral()`・シフト・`pow(k)` は元の長さを保ちます。例えば `light_fps{1,1}.pow(3)` は `{1,3}` なので、4 係数必要なら先に `resize(4)` してください。

`pre(n)` は高次の係数を切り落とすだけで、長さを伸ばすときは `resize(n)` を使います。加減算は長い方の入力長、スカラー加減算は空列なら長さ 1 になり、それ以外は元の長さを保ちます。`divmod` は `{商,余り}` の `pair` を返し、どちらも末尾の零を除き、零多項式は空配列です。`f/g` は多項式の商ではなく級数の除算です。

| 操作 | 内容・条件 |
| --- | --- |
| `light_fps()` / `light_fps(n)` / `light_fps{a0,a1,...}` / `light_fps(vector<mint>)` | 空列／零列／係数列から構築 |
| `f = a` / `f = {a0,a1,...}` | 配列／初期値列の代入 |
| `LightFormalPowerSeries<T>::multiply(f,g)` / `f * g` | 多項式の積 |
| `-f` / `f + g` / `f - g` | 符号反転／加減算 |
| `f + c` / `f - c` / `f * c` / `f / c` | 定数項への加減算／全係数の定数倍・除算。除数は非零 |
| `+=` / `-=` / `*=` / `/=` / `%=` | 対応する演算結果を自身へ代入 |
| `f << d` / `f >> d` / `f <<= d` / `f >>= d` | 係数をずらして元の長さで打ち切る。`d >= 0` |
| `f.pre(n)` | 先頭から高々 `n` 係数を返す。`n >= 0` |
| `f.shrink()` | 末尾の零を除く。零多項式は空配列になる |
| `f / g` / `f.div_series(g,deg = -1)` | 級数除算。$$g_0\ne0$$ |
| `f.divmod(g)` / `f % g` | 多項式の商と余り。除数は零多項式でない |
| `f.inv(deg = -1)` | 逆数。$$f_0\ne0$$ |
| `f.log(deg = -1)` | 対数。$$f_0=1$$ |
| `f.exp(deg = -1)` | 指数。$$f_0=0$$ |
| `f.sqrt(deg = -1)` | 平方根。存在しなければ空配列 |
| `f.pow(k)` | `k >= 0` の整数乗。`k` は `long long` |
| `f.diff()` / `f.integral()` | 微分／定数項 0 の積分 |
| `f.eval(x)` | 一点評価 |
| `f.onemul(d,c)` / `f.onediv(d,c)` | 自身に $$1+cx^d$$ を掛ける／割る。返り値は `void`。`d >= 0`。除算で `d = 0` なら `c != -1` |

指定長 0 の `inv`・`log`・`exp`・`sqrt`・`div_series` は空配列です。正の長さの `inv`・`log` には非空の入力が必要です。空列の `exp(n)` は長さ `n` の `{1,0,...}` です。`pow(0)` は元の長さの `{1,0,...}` を返し、元が空列なら空列です。

平方根は入力を `deg` 係数で打ち切って扱い、先頭の非零係数の平方根のうち通常の整数値が小さい方を選びます。先頭の非零係数の次数が奇数、または係数が平方非剰余なら空配列を返します。零級数の平方根は指定長の零列です。

`inv`・`log`・`exp`・`sqrt`・`pow`・`div_series` の出力長は $$2^{22}$$ 以下です。`divmod` は末尾の零を除いた被除数の係数数を $$2^{22}$$ 以下にします。変換長は最大 $$2^{23}$$ です。各操作の途中の積も含め、必要な NTT 長がこの上限以内であることが必要です。法 998244353 の整数の逆元を用いるため、積分・対数・指数などの次数は法未満にします。

Taylor shift・Bostan–Mori・線形漸化式・多点評価・補間は [軽量 FPS の追加演算](https://qandori.github.io/cp-library/fps/fps_light_algorithms.hpp.html) にあります。合成・合成逆元・power projection・多項式 GCD も、それぞれ `fps/compose_light.hpp`・`fps/compositional_inverse_light.hpp`・`fps/power_projection_light.hpp`・`fps/polynomial_gcd_light.hpp` にあります。いずれも軽量 FPS だけを読み込み、`light_fps_algorithms` 名前空間で利用します。

最大の入力・出力係数数を $$n$$ として、加減算・スカラー演算・微分・積分・シフト・一点評価は $$O(n)$$、積・級数除算・多項式除算・逆数・対数・指数・平方根は $$O(n\log n)$$、冪は $$O(n\log n+\log k)$$ 時間です。各操作の追加空間は $$O(n)$$ です。共有する NTT の根の表は、これまでに使用した最大変換長を $$L$$ として $$O(L)$$ 空間を保持します。整数逆元の表も、それまでに要求した最大の長さに応じて領域を保持します。
