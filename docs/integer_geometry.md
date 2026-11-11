---
title: "整数幾何"
documentation_of: //integer_geometry/geometry.hpp
---

整数・有理数を丸めずに扱う平面幾何の一括 include です。通常の [幾何](geometry.md) とは別の `integer_geometry` 名前空間を使います。

```cpp
#include "lib/integer_geometry/geometry.hpp"
#include <cassert>
#include <vector>

int main() {
	using namespace integer_geometry;
	Integer offset = Integer(1) << 100;
	std::vector<Point> points{ {offset, offset}, {offset + 3, offset}, {offset, offset + 4} };
	auto hull = convex_hull(points);
	assert(area(hull) == 6);
	auto center = centroid(hull);
	assert(center && center->x == Rational(offset) + 1);
	assert(center->y == Rational(offset) + Rational(4) / 3);
	auto circle = circumcircle(points[0], points[1], points[2]);
	assert(circle && contains(*circle, points[0]) == Location::boundary);
}
```

`Point` の各座標は任意精度整数 `Integer`、`RPoint` の各座標は任意精度有理数 `Rational` です。符号付き64 bitの端点を含む整数入力や、差・積が固定幅整数を超える中間値も扱えます。Boost.Multiprecision のヘッダーが必要で、追加のリンクライブラリは不要です。

| ヘッダー | 主な操作 |
| --- | --- |
| [point.hpp](integer_geometry_point.md) | 点・ベクトル、内積・外積、向き、任意の中心についての偏角順 |
| [linear.hpp](integer_geometry_linear.md) | 直線・線分の判定、交点、射影・反射、距離二乗 |
| [polygon.hpp](integer_geometry_polygon.md) | 面積、内外判定、重心、凸多角形の切断 |
| [convex.hpp](integer_geometry_convex.md) | 凸包、凸多角形の内外、最近・最遠点対、Minkowski 和 |
| [circle.hpp](integer_geometry_circle.md) | 半径二乗で表す円、内外・交差・位置関係、外接円、最小包含円 |
| [lattice.hpp](integer_geometry_lattice.md) | 方向の原始化、線分・多角形の格子点数 |

偏角順や向きの比較に角度・許容誤差を使いません。交点や射影の座標、面積、重心など、整数にならない値は有理数で返します。整数点の除算も有理数の点を返します。

一般の距離そのものや円の交点座標は、有理数で表せるとは限りません。距離の比較には距離二乗を使い、円は中心と半径二乗で表します。近似の角度・距離・回転・円交点などが必要な場合は通常の幾何を選びます。

計算量は個別ページを参照してください。四則演算・比較の回数に加えて、整数や有理数の分子・分母のビット長に応じた演算時間と保存空間が必要です。
