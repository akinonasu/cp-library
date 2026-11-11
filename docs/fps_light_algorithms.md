---
title: "軽量 FPS の追加演算"
documentation_of: //fps/fps_light_algorithms.hpp
---

FPS の本体と追加演算は、同じ版のヘッダーを選んでください。

```cpp
#include "lib/fps/fps_light_algorithms.hpp"

fps f{1, 2, 3};
auto shifted = taylor_shift(f, mint(2)); // {17,14,3}
std::vector<mint> xs{0, 1, 2};
auto ys = multipoint_eval(f, xs); // {1,6,17}
auto restored = interpolate(xs, ys); // {1,2,3}

std::vector<mint> initial{0, 1}, coefficients{1, 1};
auto value = linear_recurrence(initial, coefficients, 10); // 55
```

[軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) の追加演算です。通常版と同じ関数名で呼び出します。同じ機能は通常版か軽量版のいずれか一方を include してください。法は 998244353 で、係数型は `FormalPowerSeries<T>` と同じ条件です。

| 関数 | 内容・条件 |
| --- | --- |
| `taylor_shift(f,c)` | $$f(x+c)$$ の係数列を `FormalPowerSeries<T>` で返す |
| `bostan_mori(p,q,k)` | $$[x^k]p(x)/q(x)$$ を返す。$$q_0\ne0$$、`k >= 0` |
| `linear_recurrence(initial,c,k)` | $$a_n=c_0a_{n-1}+\cdots+c_{d-1}a_{n-d}$$ の $$a_k$$ を返す。初期値・係数は各 `d` 個、`k >= 0` |
| `multipoint_eval(f,xs)` | `xs` の順に値を `std::vector<T>` で返す。点の重複を許す |
| `interpolate(xs,ys)` | $$f(x_i)=y_i$$ を満たす次数が点数未満の係数列を `FormalPowerSeries<T>` で返す |

係数列は定数項から順です。線形漸化式の `c[0]` は一つ前の項に掛ける係数、`k` は 0 始まりの項番号です。`initial` と `c` は `std::vector<T>` で渡します。次数 0 の漸化式は 0 を返します。

多点評価は入力順を保ち、空の点列では空配列です。補間は `xs` と `ys` の長さが等しく、`xs` が法上で相異なることが必要です。空の補間は空配列、補間結果の長さは点数です。入力の点をソートする必要はありません。

Taylor shift では階乗と逆階乗、評価と補間では [積木](https://qandori.github.io/cp-library/components/fps_light_product_tree.hpp.html) を使います。演算中に必要となる NTT の長さは $$2^{23}$$ 以下である必要があります。

合成・合成逆元・射影・多項式 GCD は `fps/compose_light.hpp`・`fps/compositional_inverse_light.hpp`・`fps/power_projection_light.hpp`・`fps/polynomial_gcd_light.hpp` に分けています。同じ軽量 FPS を使い、通常版と同じ関数名で必要な演算だけを追加できます。

係数数を $$n$$、点数を $$m$$、Bostan–Mori の分子・分母の最大係数数または漸化式の次数を $$d$$ とします。Taylor shift は $$O(n\log n)$$ 時間・$$O(n)$$ 追加空間、Bostan–Mori と線形漸化式は $$O(d\log d\log k)$$ 時間・$$O(d)$$ 追加空間です。多点評価は $$O((n+m)\log^2(n+m))$$、補間は $$O(m\log^2 m)$$ 時間で、追加空間はそれぞれ $$O((n+m)\log(n+m))$$、$$O(m\log m)$$ です。
