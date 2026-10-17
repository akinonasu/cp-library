---
title: "modint998244353"
documentation_of: //math/mint.hpp
---

```cpp
#include "lib/math/mint.hpp"

mint x = 3;
auto power = x.pow(5);
auto inverse = x.inv();
auto value = x.val();
```

| 操作 | 内容・返り値 |
| --- | --- |
| `mint()` / `mint(x)` | 0／整数を mod 998244353 で正規化して構築 |
| `mint::mod()` / `a.val()` | 法／標準表現の値 |
| `mint::raw(x)` | 法未満の非負整数から構築 |
| `+a` / `-a` | 単項符号 |
| `a + b` / `a - b` / `a * b` / `a / b` | 四則演算。整数との演算も利用可。除数は非零 |
| `a += b` / `a -= b` / `a *= b` / `a /= b` | 四則演算の結果を代入 |
| `++a` / `a++` / `--a` / `a--` | 1 を加算／減算 |
| `a == b` / `a != b` | 等値／非等値 |
| `a.pow(k)` / `a.inv()` | 非負整数乗／逆数。逆数は非零のみ |
| `in >> a` / `out << a` | 整数の入力／標準表現の出力 |
| `mint::word(x)` / `mint::reduce(x)` | 内部の Montgomery 表現の構築／縮約。通常は `mint(x)`・`val()` を使う |

`pow(n)` の指数は非負、`inv()` と除算の除数は非零とします。

`pow(k)` は $$O(\log k)$$、その他の演算は mod 固定のため $$O(1)$$ 時間です。追加空間は $$O(1)$$ です。
