---
title: "整数幾何の点・ベクトル・偏角順"
documentation_of: //integer_geometry/point.hpp
---

```cpp
#include "lib/integer_geometry/point.hpp"
#include <cassert>
#include <vector>

int main() {
	using namespace integer_geometry;
	Integer offset = Integer(1) << 100;
	Point a{offset, offset}, b{offset + 3, offset}, c{offset, offset + 4};
	assert(cross(a, b, c) == 12 && orientation(a, b, c) == 1);
	auto middle = (a + b) / Integer(2);
	assert(middle.x == Rational(offset) + Rational(3) / 2);

	std::vector<Point> points{ {0, -1}, {-1, 0}, {2, 0}, {0, 0}, {0, 1}, {1, 0} };
	polar_sort(points);
	assert((points == std::vector<Point>{ {0, 0}, {1, 0}, {2, 0}, {0, 1}, {-1, 0}, {0, -1} }));
	std::vector<Point> around{ {9, 10}, {10, 11}, {11, 10} };
	polar_sort(around, Point{10, 10});
	assert((around.front() == Point{11, 10}));
}
```

`integer_geometry` 名前空間を使います。Boost.Multiprecision のヘッダーが必要で、追加のリンクライブラリは不要です。

| 型 | 内容 |
| --- | --- |
| `Integer` | `boost::multiprecision::cpp_int`。任意精度の符号付き整数 |
| `Rational` | `boost::multiprecision::cpp_rational`。任意精度整数の分子・分母を持つ有理数 |
| `Coordinate<T>` | `T` が `Integer` または `Rational` であることを表す概念 |
| `BasicPoint<T>` | 座標 `T x,y` を持つ点。`T` は `Coordinate` |
| `Point` / `RPoint` | `BasicPoint<Integer>` / `BasicPoint<Rational>` |
| `Location` | `outside = -1`、`boundary = 0`、`inside = 1` |

組み込み整数を `Point{x,y}` に渡すと任意精度整数の座標になります。浮動小数点の座標やスカラーは直接渡せません。`Rational(1) / 3` は厳密な3分の1ですが、C++の `1 / 3` は整数除算なので0です。`numerator(r)`・`denominator(r)` で有理数の分子・分母を取り出せます。

| 操作 | 内容・返り値 |
| --- | --- |
| `BasicPoint<T>()` / `BasicPoint<T>(x,y)` | 原点／指定した座標で構築 |
| `RPoint(p)` | 整数点を有理数点へ明示的に変換 |
| `p.x`, `p.y` | 座標 |
| `p == q`, `p < q` | 完全一致、x・y の辞書順 |
| `p + q`, `p - q`, `-p` | 同じ座標型の点の加減算 |
| `p * c`, `c * p` | 座標型 `T` のスカラーとの積。点の型を保つ |
| `p / c` | 0でない `Rational` のスカラーで割り、`RPoint` を返す。整数の除数も使用可 |
| `p += q`, `p -= q`, `p *= c` | 対応する代入演算 |
| `p /= c` | `RPoint` に対してのみ使用可。0でない有理数で割る |
| `in >> p`, `out << p` | `x y` 形式の入出力 |
| `to_rational(p)` | 値を保った `RPoint` に変換 |
| `sign(x)` | 負・0・正を `-1,0,1` で返す |
| `dot(a,b)`, `cross(a,b)` | 内積・外積を座標型 `T` で返す |
| `cross(a,b,c)` | `(b-a)` と `(c-a)` の外積を `T` で返す |
| `norm2(p)`, `distance_squared(a,b)` | 原点／2点間の距離二乗を `T` で返す |
| `orientation(a,b,c)` | a→b から見て c が左・同一直線・右なら `1,0,-1` |
| `ccw(a,b,c)` | 下記の5種類の位置関係 |
| `rotate90(p)` | 原点を中心に反時計回りに90度回転。点の型を保つ |
| `PolarLess<T>{}(a,b)` / `PolarLess<T>{center}(a,b)` | `center` を基準とする偏角順の比較 |
| `polar_sort(points,center = {})` | `vector<BasicPoint<T>>&` を偏角順に並べ替える。返り値は `void` |

同じ操作へ渡す点の座標型は揃えます。整数点と有理数点を組み合わせる場合は、整数点を `to_rational` で変換できます。

`ccw(a,b,c)` は、左が `1`、右が `-1`、a より後方が `2`、b より先が `-2`、線分上が `0` です。a と b が同じ場合、その点自身は `0`、それ以外は `-2` です。

偏角順は正のx方向から反時計回りで、`center` 自身を最初に置きます。同方向の点は中心からの距離順です。空列・重複点も扱えます。比較には浮動小数点や許容誤差を使いません。

`polar_sort` は点数を $$n$$ として $$O(n\log n)$$ 回の比較を行い、$$O(\log n)$$ 個の点を作業領域に保持します。それ以外の各操作は定数回の数値演算を行い、定数個の数値を保持します。任意精度の演算時間と数値の保存空間は、整数や分子・分母のビット長によります。
