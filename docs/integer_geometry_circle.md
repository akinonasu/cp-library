---
title: "整数幾何の円・外接円・最小包含円"
documentation_of: //integer_geometry/circle.hpp
---

```cpp
#include "lib/integer_geometry/circle.hpp"
#include <cassert>
#include <vector>

int main() {
	using namespace integer_geometry;
	auto a = make_circle(Point{0, 0}, Rational(5));
	auto b = make_circle(Point{10, 0}, Rational(5));
	assert(contains(a, Point{3, 4}) == Location::boundary);
	assert(relation(a, b) == CircleRelation::external_tangent);
	assert(intersects(a, Line<Integer>{ {-1, 5}, {1, 5} }));
	auto c = circumcircle(Point{0, 0}, Point{4, 0}, Point{0, 3});
	assert(c && c->center.x == 2 && c->center.y == Rational(3) / 2);
	assert(c->radius_squared == Rational(25) / 4);
	std::vector<Point> points{ {-2, 0}, {2, 0}, {0, 1} };
	auto bound = minimum_enclosing_circle(points);
	assert(bound && bound->center == RPoint{} && bound->radius_squared == 4);
}
```

`integer_geometry` 名前空間を使います。円を中心と半径二乗の有理数で表し、平方根や許容誤差を使わずに判定します。半径自体が無理数でも、二乗が有理数なら表せます。

| 型・操作 | 内容・返り値 |
| --- | --- |
| `Circle{center,radius_squared}` | 中心 `RPoint` と非負の半径二乗 `Rational` で構築 |
| `c.center`, `c.radius_squared` | 中心、半径二乗 |
| `make_circle(center,radius)` | 整数・有理数の点と非負の半径 `Rational` から `Circle` を構築 |
| `contains(c,p)` | 円板に対する点の位置を `Location` で返す |
| `relation(a,b)` | 2円の位置関係を `CircleRelation` で返す |
| `intersects(a,b)` | 2つの円周に共通点があるかを `bool` で返す。一致する円も `true` |
| `intersects(c,line)`, `intersects(line,c)` | 円周と直線に共通点があるか |
| `intersects(c,segment)`, `intersects(segment,c)` | 円周と線分に共通点があるか |
| `circumcircle(a,b,c)` | 3点の外接円を `optional<Circle>` で返す。同一直線上・重複点を含む退化三角形なら `nullopt` |
| `minimum_enclosing_circle(points,seed = 712367821)` | 全点を含む最小の円を `optional<Circle>` で返す。空入力なら `nullopt` |

既定構築は原点を中心とする半径0の円です。`.radius_squared` に半径そのものを渡さないでください。円へ渡す半径二乗は常に非負にします。
点・直線・線分の座標型は `Integer` または `Rational` で、外接円の3点も異なる型を組み合わせられます。

`contains` は円板、`intersects` は円周に対する判定です。線分全体が円板の内部なら `intersects` は `false` です。退化した直線・線分は点として扱い、その点が円周上にある場合だけ交差します。

`CircleRelation` は、中心・半径二乗が一致する `coincident`、外接する `external_tangent`、内接する `internal_tangent`、異なる2点で交わる `intersecting`、互いに離れた `separate`、一方が他方を内包して円周に共通点がない `contained` です。半径0の円が他方の円周上にある場合は `external_tangent` とします。

最小包含円の入力は `vector<BasicPoint<T>>` です。入力は変更しません。単点・同一点だけなら半径0の円を返し、全点が同一直線上でも使えます。`seed` はシャッフル順を決める `std::uint64_t` です。

一般の円交点・接点の座標、円周長、円板の面積は有理数で表せるとは限らないため、このヘッダーでは返しません。

点数を $$n$$ として、最小包含円は数値演算・比較をシャッフル順に対して期待 $$O(n)$$ 回、最悪 $$O(n^3)$$ 回行い、$$O(n)$$ 個の数値を保持します。それ以外の各操作は定数回の数値演算・比較を行い、定数個の数値を保持します。演算時間と数値の保存空間は、整数や分子・分母のビット長によります。
