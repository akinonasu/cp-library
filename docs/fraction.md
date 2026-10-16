---
title: "有理数（Fraction）"
documentation_of: //math/fraction.hpp
---

```cpp
#include "lib/math/fraction.hpp"

using F = Fraction<>;
F a(2, -6), b(5, 6);
auto sum = a + b;
auto n = sum.numerator();
auto d = sum.denominator();
auto lower = a.floor();
auto upper = a.ceil();
auto inverse = a.inv();
auto power = a.pow(3);
double approximation = static_cast<double>(a);
```

`Fraction<Int = std::int64_t>` は有限の分数です。`Int` は64 bit以下の標準の符号付き整数型とします。
`Fraction<>` の別名は `fraction`。常に約分し、分母は正、0 は `0/1` で保持します。
コンストラクタの引数は `Int` 型です。整数1個からの変換もできます。

四則演算・複合代入・全比較・単項符号・`++`・`--` に対応します。整数を左右どちらにも置けます。
`numerator()`・`denominator()` で分子・分母を取得します。
`floor()`・`ceil()` は数学的な切り下げ・切り上げ、`static_cast<Int>(a)` は0方向への切り捨てです。
`float`・`double`・`long double` には明示的に変換できます。
`abs(a)`・`floor(a)`・`ceil(a)`・`pow(a, k)` も使えます。`k` は `std::uint64_t` で、0乗は1です。

分母0、0による除算・逆数は不可です。**約分後の分子と正の分母が `Int` に収まること**を必要とします。
比較と四則演算の途中の積には幅の広い整数型を使うため、中間値が `Int` を超えても、約分後に収まる結果を扱えます。
`INT64_MIN` を分子に持つ値も扱えますが、その符号反転・絶対値や、正の分母が表現範囲を超える逆数は不可です。
これらの演算の前提条件は assert で検査します。

ストリーム出力は常に `分子/分母`、入力は `a/b` または整数です。分子と `/` の間に空白は置きません。
入力の整数は `Int` に収まる必要があり、分母0・表現不能な値・書式エラーでは `failbit` を立て、読み込み先を変更しません。

型の最大絶対値を $$M$$ として、構築と四則演算は $$O(\log M)$$、比較・値取得・丸めは $$O(1)$$、`pow(k)` は $$O(\log(k+1)\log M)$$ 時間です。追加空間は $$O(1)$$ です。
