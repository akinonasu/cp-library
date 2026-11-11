---
title: "整数幾何の多角形・面積・重心・切断"
documentation_of: //integer_geometry/polygon.hpp
---

```cpp
#include "lib/integer_geometry/polygon.hpp"
#include <cassert>
#include <vector>

int main() {
	using namespace integer_geometry;
	std::vector<Point> polygon{ {0, 0}, {4, 0}, {4, 3}, {0, 3} };
	assert(signed_area2(polygon) == 24 && area(polygon) == 12);
	assert(contains(polygon, Point{2, 1}) == Location::inside);
	auto center = centroid(polygon);
	assert(center && center->x == 2 && center->y == Rational(3) / 2);
	auto left = convex_cut(polygon, Line<Integer>{ {2, -1}, {2, 4} });
	assert(area(left) == 6);
}
```

`integer_geometry` 名前空間を使います。多角形は境界順の `vector<BasicPoint<T>>` で渡し、末尾に始点を重ねる必要はありません。`T` は `Integer` または `Rational` です。

| 操作 | 内容・返り値 | 演算回数 |
| --- | --- | --- |
| `signed_area2(polygon)` | 符号付き面積の2倍を座標型 `T` で返す。反時計回りが正 | $$O(n)$$ |
| `area(polygon)` | 面積の絶対値を `Rational` で返す | $$O(n)$$ |
| `contains(polygon,p)` | 点の位置を `Location::outside`・`boundary`・`inside` で返す。点と多角形の座標型は異なっていても可 | $$O(n)$$ |
| `is_convex(polygon,strict = false)` | 境界順の頂点列が凸かを `bool` で返す。`strict` が真なら辺上の余分な頂点を認めない | $$O(n)$$ |
| `centroid(polygon)` | 領域の重心を `optional<RPoint>` で返す。面積0なら `nullopt` | $$O(n)$$ |
| `convex_cut(polygon,line)` | 有向直線 `line.a -> line.b` の左側を残し、`vector<RPoint>` で返す。直線上も残す | $$O(n)$$ |

面積・重心・内外判定には単純多角形を時計回り・反時計回りのどちらでも渡せます。重心は頂点の平均ではなく領域の重心です。自己交差の検査はしません。
空・単点・線分の面積は0、重心は `nullopt` です。空集合の内外判定は `outside`、単点・線分ではその上の点だけが `boundary` です。`is_convex` は3頂点未満では `!strict` を返します。

切断には凸な境界順の頂点列と、異なる2点からなる直線を渡します。多角形と直線は異なる座標型も使えます。
返り値は空・単点・線分になることもあります。隣り合う同一点と末尾の始点の重複は除きます。返された有理数座標の頂点列を、そのまま次の切断や面積計算に使えます。

周長は一般に無理数の和となるため、このヘッダーでは返しません。

頂点数を $$n$$ として各操作は $$O(n)$$ 回の数値演算・比較を行います。`signed_area2`・`area` は定数個、それ以外は $$O(n)$$ 個の数値を保持します。演算時間と数値の保存空間は、整数や分子・分母のビット長によります。
