#include "geometry/exact_polygon.hpp"
#include "geometry/convex.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

using namespace geometry;

template<class T>
concept HasExactPolygon = requires(std::vector<Point<T>> polygon, Line<T> line) {
	area_exact(polygon);
	centroid_exact(polygon);
	convex_cut_exact(polygon, line);
};
static_assert(HasExactPolygon<std::int64_t> && HasExactPolygon<__int128_t>);
static_assert(HasExactPolygon<Integer> && HasExactPolygon<Rational>);
static_assert(!HasExactPolygon<float> && !HasExactPolygon<double> && !HasExactPolygon<Real>);

Rational reference_area(const std::vector<IPoint> &polygon) {
	__int128_t twice = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		auto a = polygon[i], b = polygon[(i + 1) % polygon.size()];
		twice += __int128_t(a.x) * b.y - __int128_t(a.y) * b.x;
	}
	return Rational(twice < 0 ? -twice : twice) / 2;
}

std::optional<RPoint> reference_centroid(const std::vector<IPoint> &polygon) {
	__int128_t twice = 0, x = 0, y = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		auto a = polygon[i], b = polygon[(i + 1) % polygon.size()];
		__int128_t weight = __int128_t(a.x) * b.y - __int128_t(a.y) * b.x;
		twice += weight;
		x += (__int128_t(a.x) + b.x) * weight;
		y += (__int128_t(a.y) + b.y) * weight;
	}
	if (!twice) return std::nullopt;
	return RPoint{Rational(x) / Rational(3 * twice), Rational(y) / Rational(3 * twice)};
}

std::vector<RPoint> reference_cut(const std::vector<IPoint> &polygon, Line<std::int64_t> line) {
	std::vector<RPoint> result;
	__int128_t a = __int128_t(line.b.y) - line.a.y;
	__int128_t b = __int128_t(line.a.x) - line.b.x;
	__int128_t c = a * line.a.x + b * line.a.y;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		auto p = polygon[i], q = polygon[(i + 1) % polygon.size()];
		__int128_t sp = c - a * p.x - b * p.y, sq = c - a * q.x - b * q.y;
		if (sp >= 0) result.push_back(RPoint{Rational(p.x), Rational(p.y)});
		if ((sp < 0 && sq > 0) || (sp > 0 && sq < 0)) {
			__int128_t d = __int128_t(q.y) - p.y, e = __int128_t(p.x) - q.x;
			__int128_t f = d * p.x + e * p.y, denominator = a * e - d * b;
			result.push_back({Rational(c * e - f * b) / Rational(denominator),
				Rational(a * f - d * c) / Rational(denominator)});
		}
	}
	std::sort(result.begin(), result.end());
	result.erase(std::unique(result.begin(), result.end()), result.end());
	return result;
}

void same_points(std::vector<RPoint> actual, std::vector<RPoint> expected) {
	std::sort(actual.begin(), actual.end());
	actual.erase(std::unique(actual.begin(), actual.end()), actual.end());
	std::sort(expected.begin(), expected.end());
	expected.erase(std::unique(expected.begin(), expected.end()), expected.end());
	assert(actual == expected);
}

int main() {
	std::vector<IPoint> empty, point{{2, 3}}, segment{{-2, 0}, {3, 0}}, collinear{{-3, 1}, {0, 1}, {5, 1}};
	for (const auto &polygon : {empty, point, segment, collinear}) {
		assert(area_exact(polygon) == 0);
		assert(!centroid_exact(polygon));
	}
	Line<std::int64_t> upper{{0, 0}, {1, 0}}, left{{0, 0}, {0, 1}};
	assert(convex_cut_exact(empty, upper).empty());
	assert(convex_cut_exact(point, upper).size() == 1);
	assert(convex_cut_exact(point, left).empty());
	same_points(convex_cut_exact(segment, left), {{-2, 0}, {0, 0}});
	same_points(convex_cut_exact(collinear, left), {{-3, 1}, {0, 1}});
	std::vector<IPoint> concave{{0, 0}, {4, 0}, {4, 1}, {1, 1}, {1, 4}, {0, 4}};
	assert(area_exact(concave) == 7);
	assert((centroid_exact(concave) == RPoint{Rational(19) / 14, Rational(19) / 14}));
	std::reverse(concave.begin(), concave.end());
	assert(area_exact(concave) == 7);
	assert((centroid_exact(concave) == RPoint{Rational(19) / 14, Rational(19) / 14}));

	std::vector<RPoint> triangle{{Rational(1) / 2, Rational(1) / 3},
		{Rational(5) / 2, Rational(1) / 3}, {Rational(1) / 2, Rational(7) / 3}};
	assert(area_exact(triangle) == 2);
	assert((centroid_exact(triangle) == RPoint{Rational(7) / 6, 1}));
	std::reverse(triangle.begin(), triangle.end());
	assert(area_exact(triangle) == 2);
	assert((centroid_exact(triangle) == RPoint{Rational(7) / 6, 1}));

	std::vector<IPoint> rectangle{{-4, -3}, {5, -3}, {5, 6}, {-4, 6}};
	auto mixed = convex_cut_exact(rectangle, Line<std::int64_t>{{2, -1}, {2, 1}});
	mixed = convex_cut_exact(mixed, Line<std::int64_t>{{-1, 1}, {1, 1}});
	assert(area_exact(mixed) == 30);
	assert((centroid_exact(mixed) == RPoint{-1, Rational(7) / 2}));
	auto clipped = convex_cut_exact(rectangle, Line<Rational>{{Rational(1) / 3, 0}, {Rational(1) / 3, 1}});
	clipped = convex_cut_exact(clipped, Line<Rational>{{0, Rational(2) / 5}, {1, Rational(2) / 5}});
	assert(area_exact(clipped) == Rational(364) / 15);
	assert((centroid_exact(clipped) == RPoint{-Rational(11) / 6, Rational(16) / 5}));
	assert(contains(clipped, RPoint{0, 1}) == Location::inside);
	assert(contains(clipped, RPoint{1, 1}) == Location::outside);
	assert(contains(clipped, RPoint{Rational(1) / 3, 1}) == Location::boundary);

	Integer shift = Integer(1) << 200;
	std::vector<BigPoint> large{{shift, shift}, {shift + 4, shift}, {shift + 4, shift + 3}, {shift, shift + 3}};
	assert(area_exact(large) == 12);
	assert((centroid_exact(large) == RPoint{Rational(shift + 2), Rational(shift) + Rational(3) / 2}));
	auto large_cut = convex_cut_exact(large, Line<Integer>{{shift + 1, shift - 1}, {shift + 1, shift + 4}});
	assert(area_exact(large_cut) == 3);
	assert((centroid_exact(large_cut) == RPoint{Rational(shift) + Rational(1) / 2, Rational(shift) + Rational(3) / 2}));
	auto low = std::numeric_limits<std::int64_t>::min(), high = std::numeric_limits<std::int64_t>::max();
	std::vector<IPoint> limits{{low, low}, {high, low}, {high, high}, {low, high}};
	Rational width = Rational(high) - low;
	assert(area_exact(limits) == width * width);
	assert((centroid_exact(limits) == RPoint{-Rational(1) / 2, -Rational(1) / 2}));
	__int128_t wide = __int128_t(1) << 100;
	std::vector<Point<__int128_t>> wide_triangle{{wide, wide}, {wide + 6, wide}, {wide, wide + 3}};
	assert(area_exact(wide_triangle) == 9);
	assert((centroid_exact(wide_triangle) == RPoint{Rational(wide) + 2, Rational(wide) + 1}));

	std::mt19937_64 random(4138791);
	for (int trial = 0; trial < 300; ++trial) {
		std::vector<IPoint> cloud;
		for (int i = 0; i < 15; ++i) cloud.push_back({std::int64_t(random() % 31) - 15, std::int64_t(random() % 31) - 15});
		auto polygon = convex_hull(cloud);
		if (trial & 1) std::reverse(polygon.begin(), polygon.end());
		assert(area_exact(polygon) == reference_area(polygon));
		assert(centroid_exact(polygon) == reference_centroid(polygon));
		Line<std::int64_t> line;
		do {
			line = {{std::int64_t(random() % 41) - 20, std::int64_t(random() % 41) - 20},
				{std::int64_t(random() % 41) - 20, std::int64_t(random() % 41) - 20}};
		} while (line.a == line.b);
		auto cut = convex_cut_exact(polygon, line);
		same_points(cut, reference_cut(polygon, line));
		for (const auto &p : cut) assert(orientation(to_rational(line.a), to_rational(line.b), p) >= 0);
		assert(area_exact(cut) <= area_exact(polygon));
		if (cut.size() >= 3 && area_exact(cut) != 0)
			assert(sign(signed_area2(cut)) == sign(signed_area2(polygon)));
	}
}
