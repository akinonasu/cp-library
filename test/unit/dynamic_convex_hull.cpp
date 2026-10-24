#include "geometry/dynamic_convex_hull.hpp"
#include "geometry/convex.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <vector>
using namespace geometry;
void check(const DynamicConvexHull<> &h, const std::vector<IPoint> &points, std::mt19937_64 &random) {
	auto expected = convex_hull(points);
	assert(h.vertices() == expected);
	assert(h.size() == expected.size());
	__int128_t area = 0;
	for (std::size_t i = 0; i < expected.size(); ++i) area += cross(expected[i], expected[(i + 1) % expected.size()]);
	assert(h.doubled_area() == area);
	for (int j = 0; j < 30; ++j) {
		IPoint p{std::int64_t(random() % 101) - 50, std::int64_t(random() % 101) - 50};
		assert(h.contains(p) == convex_contains(expected, p));
		if (points.empty()) { assert(!h.max_dot(p) && !h.min_dot(p) && !h.support(p)); continue; }
		auto low = dot(points[0], p), high = low;
		for (auto q : points) low = std::min(low, dot(q, p)), high = std::max(high, dot(q, p));
		assert(h.min_dot(p) == low && h.max_dot(p) == high);
		assert(dot(*h.support(p), p) == high);
	}
	for (auto p : points) assert(h.contains(p) != Location::outside);
}
int main() {
	std::mt19937_64 random(42421);
	for (int kind = 0; kind < 6; ++kind) for (int trial = 0; trial < 30; ++trial) {
		DynamicConvexHull<> hull;
		std::vector<IPoint> points;
		check(hull, points, random);
		for (int i = 0; i < 100; ++i) {
			IPoint p{std::int64_t(random() % 61) - 30, std::int64_t(random() % 61) - 30};
			if (kind == 1) p.y = p.x;
			if (kind == 2) p.x = 0;
			if (kind == 3) p.y = 0;
			if (kind == 4) p.x = i - 50;
			if (kind == 5) p = {1, 2};
			bool change = hull.insert(p);
			points.push_back(p);
			auto previous = convex_hull(std::vector<IPoint>(points.begin(), points.end() - 1));
			assert(change == (previous != convex_hull(points)));
			check(hull, points, random);
		}
		auto copy = hull;
		hull.clear();
		check(copy, points, random);
		auto moved = std::move(copy);
		copy.insert({0, 0});
		check(copy, {{0, 0}}, random);
		check(moved, points, random);
	}
	std::vector<IPoint> parabola;
	for (int x = -2048; x < 2048; ++x) parabola.push_back({x, std::int64_t(x) * x});
	std::shuffle(parabola.begin(), parabola.end(), random);
	DynamicConvexHull<> many(parabola);
	assert(many.vertices() == convex_hull(parabola));
	for (int x = -2048; x < 2048; ++x) assert(many.max_dot({2 * x, -1}) == __int128_t(x) * x);
	for (auto p : std::vector<IPoint>{{-100000000, -100000000}, {100000000, -100000000}, {100000000, 100000000}, {-100000000, 100000000}}) many.insert(p);
	assert(many.size() == 4);
	auto copied = many;
	auto *self = &copied;
	copied = *self;
	many = std::move(copied);
	assert(copied.empty() && many.size() == 4);
	copied.insert({1, 1});
	assert(copied.size() == 1);
	constexpr std::int64_t b = 1000000000000000000LL;
	DynamicConvexHull<> wide({{-b, -b}, {b, -b}, {b, b}, {-b, b}});
	assert(wide.support({0, 0}));
	assert(wide.doubled_area() == __int128_t(8) * b * b);
	assert(wide.max_dot({b, b}) == __int128_t(2) * b * b);
	assert(wide.contains({0, 0}) == Location::inside);
	DynamicConvexHull<int> narrow({{INT32_MIN, INT32_MIN}, {INT32_MAX, INT32_MAX}});
	assert(narrow.min_dot({INT32_MIN, 0}) == __int128_t(INT32_MIN) * INT32_MAX);
}
