#include "geometry/rational.hpp"
#include "geometry/convex.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <type_traits>
#include <vector>

using namespace geometry;
static_assert(std::is_same_v<wide_t<Integer>, Integer>);
static_assert(std::is_same_v<wide_t<Rational>, Rational>);
static_assert(ExactCoordinate<std::int64_t> && ExactCoordinate<__int128_t>);
static_assert(ExactCoordinate<Integer> && ExactCoordinate<Rational>);
static_assert(!ExactCoordinate<Real> && !ExactCoordinate<unsigned>);

template<class T>
void check_pairs(const std::vector<Point<T>> &points) {
	wide_t<T> nearest = distance_squared(points[0], points[1]), furthest = nearest;
	for (std::size_t i = 0; i < points.size(); ++i) for (std::size_t j = 0; j < i; ++j) {
		auto d = distance_squared(points[i], points[j]);
		nearest = std::min(nearest, d);
		furthest = std::max(furthest, d);
	}
	auto near = *closest_pair(points), far = *furthest_pair(points);
	assert(near.first != near.second && far.first != far.second);
	assert(near.squared_distance == nearest && far.squared_distance == furthest);
	assert(distance_squared(points[near.first], points[near.second]) == nearest);
	assert(distance_squared(points[far.first], points[far.second]) == furthest);
	assert(diameter_squared(convex_hull(points)) == furthest);
}

int main() {
	Integer large = Integer(1) << 200;
	BigPoint a{large, large}, b{large + 1, large}, c{large, large + 1};
	assert(orientation(a, b, c) == 1 && cross(a, b, c) == 1);
	assert(dot(BigPoint{large, 0}, BigPoint{large, 0}) == large * large);
	assert(distance_squared(a, c) == 1);
	assert(almost_equal(a, a) && !almost_equal(a, b));
	assert(ccw(a, b, BigPoint{large + 2, large}) == -2);
	assert((rotate90(BigPoint{large, 1}) == BigPoint{-1, large}));
	std::vector<BigPoint> directions{{0, 0}, {large, 1}, {large - 1, 1}, {-1, large}, {-large, -1}, {1, -large}};
	polar_sort(directions);
	assert((directions[0] == BigPoint{} && directions[1] == BigPoint{large, 1}));
	assert((directions[2] == BigPoint{large - 1, 1}));
	Rational small = Rational(1) / Rational(Integer(1) << 500);
	RPoint r{small, small}, s{2 * small, small}, t{small, 2 * small};
	assert(sign(small) == 1 && sign(-small) == -1);
	assert(orientation(r, s, t) == 1 && !almost_equal(r, s));
	assert(on_segment(Segment<Rational>{{0, 0}, {3 * small, 3 * small}}, r));
	assert(!on_segment(Segment<Rational>{{0, 0}, {small, small}}, s));
	assert(intersects(Segment<Rational>{r, s}, Segment<Rational>{r, t}));
	assert((to_rational(a) == RPoint{Rational(large), Rational(large)}));
	assert((to_rational(Point<__int128_t>{__int128_t(1) << 100, -1}).x == Rational(Integer(1) << 100)));
	std::vector<RPoint> tiny{{0, 0}, {small, 0}, {small, small}, {0, small}};
	assert(signed_area2(tiny) == 2 * small * small && is_convex(tiny, true));
	assert(contains(tiny, RPoint{small / 2, small / 2}) == Location::inside);
	assert(convex_contains(tiny, RPoint{small, small / 2}) == Location::boundary);
	auto sum = minkowski_sum(tiny, tiny);
	assert(sum.size() == 4 && signed_area2(sum) == 8 * small * small);
	std::mt19937 random(71823);
	for (int trial = 0; trial < 60; ++trial) {
		std::vector<IPoint> points;
		std::vector<BigPoint> big;
		std::vector<RPoint> rational;
		int count = 2 + random() % 100, denominator = 2 + random() % 17;
		for (int i = 0; i < count; ++i) {
			IPoint p{int(random() % 41) - 20, int(random() % 41) - 20};
			points.push_back(p);
			big.push_back({large + p.x, -large + p.y});
			rational.push_back({Rational(large) + Rational(p.x) / denominator, -Rational(large) + Rational(p.y) / denominator});
		}
		auto hull = convex_hull(points);
		auto big_hull = convex_hull(big);
		auto rational_hull = convex_hull(rational);
		assert(big_hull.size() == hull.size() && rational_hull.size() == hull.size());
		for (std::size_t i = 0; i < hull.size(); ++i) {
			assert((big_hull[i] == BigPoint{large + hull[i].x, -large + hull[i].y}));
			assert((rational_hull[i] == RPoint{Rational(large) + Rational(hull[i].x) / denominator,
				-Rational(large) + Rational(hull[i].y) / denominator}));
		}
		assert(signed_area2(big_hull) == Integer(signed_area2(hull)));
		assert(signed_area2(rational_hull) == Rational(signed_area2(hull)) / (denominator * denominator));
		auto indices = convex_hull_indices(rational);
		for (std::size_t i = 0; i < indices.size(); ++i) assert(rational[indices[i]] == rational_hull[i]);
		for (int i = 0; i < 10; ++i) {
			IPoint p{int(random() % 61) - 30, int(random() % 61) - 30};
			RPoint q{Rational(large) + Rational(p.x) / denominator, -Rational(large) + Rational(p.y) / denominator};
			assert(contains(hull, p) == contains(rational_hull, q));
			assert(convex_contains(hull, p) == convex_contains(rational_hull, q));
		}
		check_pairs(big);
		check_pairs(rational);
	}
}
