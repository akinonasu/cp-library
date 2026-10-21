#include "geometry/polygon.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

using namespace geometry;

int main() {
	std::vector<IPoint> square{{0, 0}, {4, 0}, {4, 4}, {0, 4}};
	assert(signed_area2(square) == 32 && area(square) == 16 && perimeter(square) == 16);
	assert(is_convex(square) && is_convex(square, true));
	assert(centroid(square) == FPoint(2, 2));
	for (int x = -1; x <= 5; ++x) for (int y = -1; y <= 5; ++y) {
		auto expected = x < 0 || x > 4 || y < 0 || y > 4 ? Location::outside
			: x == 0 || x == 4 || y == 0 || y == 4 ? Location::boundary : Location::inside;
		assert(contains(square, IPoint(x, y)) == expected);
	}
	std::reverse(square.begin(), square.end());
	assert(signed_area2(square) == -32 && area(square) == 16 && centroid(square) == FPoint(2, 2));
	assert(contains(square, IPoint(1, 1)) == Location::inside);
	std::vector<IPoint> concave{{0, 0}, {8, 0}, {8, 2}, {2, 2}, {2, 8}, {0, 8}};
	assert(!is_convex(concave) && signed_area2(concave) == 56);
	for (int x = -2; x <= 10; ++x) for (int y = -2; y <= 10; ++y) {
		bool inside = 0 < x && x < 8 && 0 < y && y < 8 && (x < 2 || y < 2);
		bool boundary = (y == 0 && 0 <= x && x <= 8) || (x == 0 && 0 <= y && y <= 8)
			|| (x == 8 && 0 <= y && y <= 2) || (y == 8 && 0 <= x && x <= 2)
			|| (y == 2 && 2 <= x && x <= 8) || (x == 2 && 2 <= y && y <= 8);
		assert(contains(concave, IPoint(x, y)) == (boundary ? Location::boundary : inside ? Location::inside : Location::outside));
	}
	std::vector<IPoint> collinear{{0, 0}, {1, 0}, {2, 0}};
	assert(is_convex(collinear) && !is_convex(collinear, true) && !centroid(collinear));
	std::vector<IPoint> empty, point{{2, 3}}, segment{{1, 0}, {3, 0}};
	assert(area(empty) == 0 && perimeter(empty) == 0 && !centroid(empty));
	assert(contains(empty, IPoint(0, 0)) == Location::outside);
	assert(contains(point, IPoint(2, 3)) == Location::boundary && contains(point, IPoint(2, 2)) == Location::outside);
	assert(contains(segment, IPoint(2, 0)) == Location::boundary && contains(segment, IPoint(4, 0)) == Location::outside);
	assert(perimeter(segment) == 4 && perimeter(point) == 0);
	std::vector<IPoint> translated{{1000000000000000000LL, 1000000000000000000LL},
		{1000000000000000007LL, 1000000000000000000LL}, {1000000000000000007LL, 1000000000000000005LL},
		{1000000000000000000LL, 1000000000000000005LL}};
	assert(signed_area2(translated) == 70);
	constexpr std::int64_t k = 1000000000000000000LL;
	std::vector<IPoint> thin{ {0, 0}, {k, k - 1}, {k - 1, k - 2} };
	assert(signed_area2(thin) == -1);
	auto thin_center = centroid(thin);
	assert(thin_center && std::abs(thin_center->x - (Real(k) + Real(k - 1)) / 3) <= 1e-15L * k);
	assert(std::abs(thin_center->y - (Real(k - 1) + Real(k - 2)) / 3) <= 1e-15L * k);
	std::vector<IPoint> box{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
	auto left = convex_cut(box, Line<std::int64_t>{{1, -1}, {1, 3}});
	assert(area(left) == 2 && centroid(left) == FPoint(0.5L, 1));
	assert(convex_cut(box, Line<std::int64_t>{{-1, -1}, {-1, 3}}).empty());
	assert(area(convex_cut(box, Line<std::int64_t>{{3, -1}, {3, 3}})) == 4);
	auto diagonal = convex_cut(box, Line<std::int64_t>{{0, 0}, {2, 2}});
	assert(area(diagonal) == 2 && diagonal.size() == 3);
	assert(convex_cut(empty, Line<std::int64_t>{{0, 0}, {1, 0}}).empty());
}
