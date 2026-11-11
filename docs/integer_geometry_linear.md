---
title: "整数幾何の直線・線分・交点・距離二乗"
documentation_of: //integer_geometry/linear.hpp
---

```cpp
#include "lib/integer_geometry/linear.hpp"
#include <cassert>

int main() {
	using namespace integer_geometry;
	Line<Integer> l{ {0, 0}, {3, 1} };
	Line<Integer> m{ {0, 1}, {1, 0} };
	auto p = intersection(l, m);
	assert(p && p->x == Rational(3) / 4 && p->y == Rational(1) / 4);
	assert(distance_squared(l, Point{0, 1}) == Rational(9) / 10);
	assert(on_line(l, *p));
	auto q = projection(l, Point{0, 1});
	assert(q.x == Rational(3) / 10 && q.y == Rational(1) / 10);
}
```

`integer_geometry` 名前空間を使います。座標型 `T,U` は `Integer` または `Rational` です。異なる座標型の点・直線・線分も組み合わせられます。

| 型・操作 | 内容・返り値 |
| --- | --- |
| `Line<T = Integer>{a,b}` | 2点を通る直線 |
| `Segment<T = Integer>{a,b}` | 2点を両端とする閉線分 |
| `on_line(l,p)`, `on_segment(s,p)` | 点が直線／閉線分上にあるかを `bool` で返す |
| `parallel(l,m)`, `orthogonal(l,m)` | 直線の方向が平行／直交かを `bool` で返す |
| `intersects(a,b)` | 直線・線分同士に共通点があるかを `bool` で返す |
| `intersection(l,m)` | 直線同士の唯一の交点を `optional<RPoint>` で返す。平行・一致直線は `nullopt` |
| `intersection(s,t)` | 線分同士の共通部分を `vector<RPoint>` で返す。0点・1点・重なりの両端2点 |
| `intersection(l,s)`, `intersection(s,l)` | 直線と線分の共通部分を `vector<RPoint>` で返す |
| `projection(l,p)` | 点の直線への射影を `RPoint` で返す |
| `reflection(l,p)` | 直線に関して対称な点を `RPoint` で返す |
| `distance_squared(a,b)` | 点・直線・線分の組の最短距離二乗。直線・線分を含む組では `Rational` を返す。対称な引数順も使用可 |
| `perpendicular_bisector(a,b)` | 2点の垂直二等分線を `optional<Line<Rational>>` で返す。同一点なら `nullopt` |

直線・線分の `.a,.b` は端点です。`Line(a,b)`・`Segment(a,b)` では同じ型の点から座標型を推論できます。2端点が同じ場合は点として扱います。退化直線を含む交点でも、共通部分が1点ならその点を返します。

線分同士の重なりの両端は辞書順、直線と線分の重なりは線分の `.a,.b` の順です。共通部分が1点なら重複させません。
退化直線への射影はその点を返し、反射はその点を中心とする点対称です。退化直線の方向ベクトルは零なので、`parallel`・`orthogonal` はどちらも `true` です。垂直二等分線は中点と、そこから辺に垂直な方向へ進んだ点で表します。

点同士の `distance_squared` は [point.hpp](integer_geometry_point.md) の操作で、同じ座標型 `T` の距離二乗を `T` で返します。異なる座標型の点同士は `to_rational` で揃えます。
距離そのものは一般に無理数となるため、距離二乗を返します。返された有理数座標の点は、次の交点・射影・距離二乗の入力にも使えます。

各操作は定数回の数値演算・比較を行い、定数個の数値を保持します。演算時間と数値の保存空間は、整数や分子・分母のビット長によります。
