---
title: "厳密な整数幾何の補助"
documentation_of: //components/exact_geometry_integer.hpp
library_category: components/geometry
---

`geometry::exact_integer_detail` 名前空間の内部部品です。厳密幾何のヘッダーから使われるため、通常は直接 include する必要はありません。

`Integer` は任意精度整数、`Rational` は任意精度有理数です。`BigPoint` は `Point<Integer>`、`RPoint` は `Point<Rational>` を表します。いずれも `geometry` 名前空間で定義しています。

入力座標は符号付き組み込み整数型または `geometry::Integer` とし、積や差を任意精度整数で計算します。`coordinate<T>` は `Point<T>` の座標型のうち、これらの整数型で `true` です。

| 操作 | 内容・返り値 |
| --- | --- |
| `convert(point)` | 整数座標の点を `BigPoint` へ変換 |
| `quotient(point,d)` | `BigPoint` の各座標を0でない `Integer` の `d` で割った `RPoint` |
| `projection(a,b,p)` | 射影の座標の分子と共通分母を `pair<BigPoint,Integer>` で返す。`a == b` ならその点と分母1 |
| `segment_distance(a,b,p)` | 線分 `ab` と点 `p` の距離二乗を `Rational` で返す。同一点の線分も扱う |
| `Circle{center,denominator,square}` | 中心の分子 `BigPoint`、0でない共通分母 `Integer`、非負の半径二乗の分子 `Integer` で円を表す |
| `Circle::indicator(p)` | 円板の外部なら正、円周上なら0、内部なら負の `Integer` |
| `diameter(a,b)` | `a,b` を直径の両端とする `Circle` |
| `circumcircle(a,b,c)` | 外接円を `optional<Circle>` で返す。共線・重複なら `nullopt` |

`Circle` の `center` が $$(x,y)$$、`denominator` が $$d$$、`square` が $$r$$ のとき、中心は $$(x/d,y/d)$$、半径二乗は $$r/d^2$$ です。既定構築では原点を中心とする半径0の円です。

各操作は整数・有理数の四則演算と比較を定数回行い、定数個の数値を保持します。演算の時間・保存空間は数値のビット長によります。
