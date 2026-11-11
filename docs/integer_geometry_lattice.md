---
title: "整数幾何の格子点数・方向の原始化"
documentation_of: //integer_geometry/lattice.hpp
---

```cpp
#include "lib/integer_geometry/lattice.hpp"
#include <cassert>
#include <vector>

int main() {
	using namespace integer_geometry;
	assert((primitive_direction(Point{6, -9}) == Point{2, -3}));
	assert(segment_lattice_points(Point{0, 0}, Point{6, 4}) == 3);
	std::vector<Point> polygon{ {0, 0}, {4, 0}, {4, 3}, {0, 3} };
	assert(boundary_lattice_points(polygon) == 14);
	assert(interior_lattice_points(polygon) == 6);
}
```

`integer_geometry` 名前空間を使います。格子点は両座標が整数の点です。このヘッダーの点・多角形には `Point` を渡します。

| 操作 | 内容・返り値 |
| --- | --- |
| `gcd(a,b)` | 2つの `Integer` の非負の最大公約数を `Integer` で返す。`gcd(0,0)` は0 |
| `primitive_direction(p)` | 座標を最大公約数で割った同方向の `Point`。零ベクトルなら零ベクトル |
| `segment_lattice_points(a,b)` | 両端を含む線分上の格子点数を `Integer` で返す。同一点なら1 |
| `boundary_lattice_points(polygon)` | 格子多角形の境界上の格子点数を `Integer` で返す |
| `interior_lattice_points(polygon)` | 格子多角形の内部の格子点数を `Integer` で返す |

多角形は境界順の `vector<Point>` とします。非退化な単純多角形を時計回り・反時計回りのどちらでも渡せます。末尾での始点の反復と隣り合う同一点も認めます。自己交差の検査はしません。

空入力の境界点数は0、単点は1です。全点が同一直線上なら、その両端の間の線分に含まれる格子点数を返します。空・単点・線分・全点が同一直線上の場合、内部の格子点数は0です。

内部の点数は Pick の定理で求めます。面積を $$A$$、境界上の格子点数を $$B$$ とすると、内部の格子点数は $$A-B/2+1$$ です。

点数を $$n$$ として、多角形の点数は $$O(n)$$ 回の最大公約数計算と数値演算を行い、定数個の数値を保持します。方向の原始化・線分上の点数は定数回の最大公約数計算と数値演算を行います。演算時間と数値の保存空間は整数のビット長によります。
