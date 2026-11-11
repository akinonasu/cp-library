#include "integer_geometry/geometry.hpp"
#ifdef BOOST_MP_CPP_INT_HPP
#error "Integer geometry must not include multiprecision integers"
#endif
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

namespace ig = integer_geometry;
using ig::Point;
using ig::Wide;
using ig::Integer;
using ig::Location;
using Big = boost::multiprecision::cpp_int;

Big determinant(const Point &a, const Point &b, const Point &c) {
	return (Big(b.x) - a.x) * (Big(c.y) - a.y) - (Big(b.y) - a.y) * (Big(c.x) - a.x);
}

Big square(const Point &a, const Point &b) {
	Big x = Big(a.x) - b.x, y = Big(a.y) - b.y;
	return x * x + y * y;
}

int turn(const Point &a, const Point &b, const Point &c) {
	Big value = determinant(a, b, c);
	return (value > 0) - (value < 0);
}

bool between(const Point &a, const Point &b, const Point &p) {
	return determinant(a, b, p) == 0 && std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x)
		&& std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
}

Location inside(const std::vector<Point> &polygon, const Point &p) {
	int winding = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		const auto &a = polygon[i], &b = polygon[(i + 1) % polygon.size()];
		if (between(a, b, p)) return Location::boundary;
		int value = turn(a, b, p);
		if (a.y <= p.y && p.y < b.y && value > 0) ++winding;
		if (b.y <= p.y && p.y < a.y && value < 0) --winding;
	}
	return winding ? Location::inside : Location::outside;
}

std::vector<Point> gift_wrap(std::vector<Point> points) {
	std::sort(points.begin(), points.end());
	points.erase(std::unique(points.begin(), points.end()), points.end());
	if (points.size() < 3) return points;
	std::vector<Point> result;
	int current = 0;
	do {
		result.push_back(points[current]);
		int next = current == 0 ? 1 : 0;
		for (int i = 0; i < int(points.size()); ++i) if (i != current) {
			int value = turn(points[current], points[next], points[i]);
			if (value < 0 || (value == 0 && square(points[current], points[i]) > square(points[current], points[next]))) next = i;
		}
		current = next;
		assert(result.size() <= points.size());
	} while (current != 0);
	return result;
}

bool polar_oracle(const Point &a, const Point &b, Point center = {}) {
	Big ax = Big(a.x) - center.x, ay = Big(a.y) - center.y;
	Big bx = Big(b.x) - center.x, by = Big(b.y) - center.y;
	bool zero_a = ax == 0 && ay == 0, zero_b = bx == 0 && by == 0;
	if (zero_a || zero_b) return zero_a && !zero_b;
	bool back_a = ay < 0 || (ay == 0 && ax < 0), back_b = by < 0 || (by == 0 && bx < 0);
	if (back_a != back_b) return back_a < back_b;
	Big value = ax * by - ay * bx;
	if (value != 0) return value > 0;
	return ax * ax + ay * ay < bx * bx + by * by;
}

void check_polar(std::vector<Point> points, Point center = {}) {
	auto expected = points;
	auto less = [&](const Point &a, const Point &b) { return polar_oracle(a, b, center); };
	std::sort(expected.begin(), expected.end(), less);
	ig::polar_sort(points, center);
	assert(points == expected);
	std::reverse(points.begin(), points.end());
	std::sort(points.begin(), points.end(), ig::PolarLess<Integer>{center});
	assert(points == expected);
}

template<class P>
concept HasDivision = requires(P p) { p / 2; };

void point_and_polar() {
	static_assert(ig::Coordinate<int> && ig::Coordinate<Integer>);
	static_assert(!ig::Coordinate<unsigned> && !ig::Coordinate<double> && !ig::Coordinate<Big>);
	static_assert(std::is_same_v<Integer, std::int64_t> && std::is_same_v<Wide, __int128_t>);
	static_assert(std::is_same_v<decltype(ig::dot(Point{}, Point{})), Wide>);
	static_assert(std::is_same_v<decltype(ig::distance_squared(Point{}, Point{})), Wide>);
	static_assert(!std::is_constructible_v<Point, double, double> && !HasDivision<Point>);
	constexpr Integer limit = 1000000000000000000LL;
	std::vector<Point> extremes{{-limit, -limit}, {limit, -limit}, {-limit, limit}, {limit, limit}, {limit - 1, limit}, {0, 0}};
	for (const auto &a : extremes) for (const auto &b : extremes) {
		assert(Big(ig::distance_squared(a, b)) == square(a, b));
		assert(Big(ig::dot(a, b)) == Big(a.x) * b.x + Big(a.y) * b.y);
		for (const auto &c : extremes) {
			assert(Big(ig::cross(a, b, c)) == determinant(a, b, c));
			assert(ig::orientation(a, b, c) == turn(a, b, c));
		}
	}
	Point p{11, -17};
	assert(-(-p) == p && p + Point(1, 2) - Point(1, 2) == p);
	assert(p * Integer(3) == Integer(3) * p && ig::rotate90(p) == Point(17, 11));
	Point scaled{2, 3};
	scaled *= scaled.x;
	assert(scaled == Point(4, 6));
	std::stringstream stream;
	stream << p;
	Point restored;
	stream >> restored;
	assert(restored == p);
	assert(ig::ccw(Point(0, 0), Point(2, 0), Point(0, 1)) == 1);
	assert(ig::ccw(Point(0, 0), Point(2, 0), Point(0, -1)) == -1);
	assert(ig::ccw(Point(0, 0), Point(2, 0), Point(-1, 0)) == 2);
	assert(ig::ccw(Point(0, 0), Point(2, 0), Point(3, 0)) == -2);
	assert(ig::ccw(Point(0, 0), Point(2, 0), Point(1, 0)) == 0);
	assert(ig::ccw(Point{}, Point{}, Point{}) == 0 && ig::ccw(Point{}, Point{}, Point(1, 0)) == -2);
	assert(ig::sign(Wide(-limit) * limit) == -1 && ig::sign(0) == 0 && ig::sign(limit) == 1);
	std::vector<Point> vectors;
	for (int x = -3; x <= 3; ++x) for (int y = -3; y <= 3; ++y) vectors.emplace_back(x, y);
	vectors.insert(vectors.end(), {{limit, 1}, {limit - 1, 1}, {limit, limit - 1}, {limit - 1, limit}, {0, 0}});
	check_polar(vectors);
	Point center{limit / 2, -limit / 2};
	for (auto &v : vectors) v += center;
	check_polar(vectors, center);
	Integer low = std::numeric_limits<Integer>::min(), high = std::numeric_limits<Integer>::max();
	vectors.clear();
	for (auto x : {low, low + 1, Integer(-1), Integer(0), Integer(1), high - 1, high})
		for (auto y : {low, low + 1, Integer(-1), Integer(0), Integer(1), high - 1, high}) vectors.emplace_back(x, y);
	check_polar(vectors);
	check_polar({});
	std::mt19937_64 random(701023);
	vectors.clear();
	for (int i = 0; i < 1000; ++i) vectors.emplace_back(Integer(random()), Integer(random()));
	check_polar(vectors);
	std::vector<ig::BasicPoint<int>> small{{0, -1}, {-1, 0}, {2, 0}, {0, 0}, {0, 1}, {1, 0}};
	ig::polar_sort(small);
	assert((small == std::vector<ig::BasicPoint<int>>{{0, 0}, {1, 0}, {2, 0}, {0, 1}, {-1, 0}, {0, -1}}));
}

void check_pairs(const std::vector<Point> &points, bool lattice) {
	auto hull = ig::convex_hull(points);
	assert(hull == gift_wrap(points));
	auto indices = ig::convex_hull_indices(points);
	assert(indices.size() == hull.size());
	for (std::size_t i = 0; i < indices.size(); ++i) assert(0 <= indices[i] && indices[i] < int(points.size()) && points[indices[i]] == hull[i]);
	auto nearest = ig::closest_pair(points), furthest = ig::furthest_pair(points);
	if (points.size() < 2) assert(!nearest && !furthest);
	else {
		Big minimum = square(points[0], points[1]), maximum = minimum;
		for (std::size_t i = 0; i < points.size(); ++i) for (std::size_t j = 0; j < i; ++j) {
			Big value = square(points[i], points[j]);
			minimum = std::min(minimum, value); maximum = std::max(maximum, value);
		}
		for (auto result : {nearest, furthest}) {
			assert(result && 0 <= result->first && result->first < int(points.size()));
			assert(0 <= result->second && result->second < int(points.size()) && result->first != result->second);
			assert(Big(result->squared_distance) == square(points[result->first], points[result->second]));
		}
		assert(Big(nearest->squared_distance) == minimum && Big(furthest->squared_distance) == maximum);
		assert(Big(ig::diameter_squared(hull)) == maximum);
	}
	auto diameter = ig::convex_diameter(hull);
	assert(bool(diameter) == !hull.empty());
	if (diameter) assert(Big(diameter->squared_distance) == square(hull[diameter->first], hull[diameter->second]));
	Big twice = 0;
	for (std::size_t i = 0; i < hull.size(); ++i) twice += Big(hull[i].x) * hull[(i + 1) % hull.size()].y - Big(hull[i].y) * hull[(i + 1) % hull.size()].x;
	assert(Big(ig::signed_area2(hull)) == twice && Big(ig::doubled_area(hull)) == twice);
	if (!lattice) return;
	for (int x = -6; x <= 6; ++x) for (int y = -6; y <= 6; ++y) {
		Point p{x, y};
		assert(ig::convex_contains(hull, p) == inside(hull, p));
		assert(ig::contains(hull, p) == inside(hull, p));
	}
	Integer count = 0, boundary = 0;
	for (int x = -6; x <= 6; ++x) for (int y = -6; y <= 6; ++y) {
		auto where = inside(hull, Point{x, y});
		count += where == Location::inside;
		boundary += where == Location::boundary;
	}
	assert(ig::boundary_lattice_points(hull) == boundary);
	assert(ig::interior_lattice_points(hull) == count);
}

void convex_and_lattice() {
	std::mt19937 random(89364013);
	for (int trial = 0; trial < 100; ++trial) {
		std::vector<Point> points;
		int n = random() % 90;
		for (int i = 0; i < n; ++i) points.emplace_back(int(random() % 11) - 5, int(random() % 11) - 5);
		check_pairs(points, true);
		std::vector<Point> other;
		for (int i = 0; i < 6; ++i) other.emplace_back(int(random() % 11) - 5, int(random() % 11) - 5);
		std::vector<Point> sums;
		for (const auto &p : points) for (const auto &q : other) sums.push_back(p + q);
		assert(ig::minkowski_sum(ig::convex_hull(points), ig::convex_hull(other)) == gift_wrap(sums));
	}
	constexpr Integer big = 1000000000000000000LL;
	std::vector<Point> points{{big, big}, {big + 3, big}, {big, big + 4}, {big + 1, big + 1}, {big, big}};
	check_pairs(points, false);
	auto hull = ig::convex_hull(points);
	assert(ig::doubled_area(hull) == 12 && ig::boundary_lattice_points(hull) == 8);
	assert(ig::interior_lattice_points(hull) == 3);
	points.clear();
	for (int i = 0; i < 80; ++i) points.emplace_back((Integer(random()) * 1000003) % big - big / 2, (Integer(random()) * 1000033) % big - big / 2);
	check_pairs(points, false);
	std::vector<Point> border{{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}, {0, 1}, {1, 1}, {0, 0}};
	auto all = ig::convex_hull(border, true);
	assert((all == std::vector<Point>{{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}, {0, 1}}));
	assert(ig::gcd(Wide(-big) * 6, Wide(big) * 9) == Wide(big) * 3);
	assert(ig::primitive_direction(Point(big * 6, -big * 9)) == Point(2, -3));
	assert(ig::primitive_direction(Point{}) == Point{});
	for (int x = -5; x <= 5; ++x) for (int y = -5; y <= 5; ++y)
		assert(ig::segment_lattice_points(Point(-2, 3), Point(x, y)) == std::gcd(x + 2, y - 3) + 1);
	assert(ig::gcd(0, 0) == 0 && ig::gcd(-7, 0) == 7);
	assert(ig::boundary_lattice_points({}) == 0 && ig::interior_lattice_points({}) == 0);
	assert(ig::boundary_lattice_points({Point(1, 2)}) == 1);
	assert(ig::boundary_lattice_points({Point(-2, -2), Point(3, 3), Point(0, 0), Point(-2, -2)}) == 6);
	assert(ig::interior_lattice_points({Point(-2, -2), Point(3, 3), Point(0, 0), Point(-2, -2)}) == 0);
	border = {{0, 0}, {2, 0}, {2, 2}, {0, 2}, {0, 0}};
	assert(ig::boundary_lattice_points(border) == 8 && ig::interior_lattice_points(border) == 1);
	std::reverse(border.begin(), border.end());
	assert(ig::signed_area2(border) == -8 && ig::doubled_area(border) == 8 && ig::interior_lattice_points(border) == 1);
	Integer low = std::numeric_limits<Integer>::min(), high = std::numeric_limits<Integer>::max();
	assert(ig::gcd(std::numeric_limits<Wide>::min(), 1) == 1);
	assert(ig::primitive_direction(Point(low, low)) == Point(-1, -1));
	assert(ig::primitive_direction(Point(low, 1)) == Point(low, 1));
	assert(ig::segment_lattice_points(Point(low, 1), Point(high, 1)) == (Wide(1) << 64));
	assert(ig::boundary_lattice_points({Point(low, 1), Point(high, 1)}) == (Wide(1) << 64));
}

void linear_and_polygon() {
	std::vector<Point> points;
	for (int x = -1; x <= 1; ++x) for (int y = -1; y <= 1; ++y) points.emplace_back(x, y);
	for (const auto &a : points) for (const auto &b : points) for (const auto &c : points) for (const auto &d : points) {
		int ac = turn(a, b, c), ad = turn(a, b, d), ca = turn(c, d, a), cb = turn(c, d, b);
		bool expected = between(a, b, c) || between(a, b, d) || between(c, d, a) || between(c, d, b)
			|| (ac * ad < 0 && ca * cb < 0);
		ig::Segment<> s{a, b}, t{c, d};
		assert(ig::intersects(s, t) == expected && ig::intersects(t, s) == expected);
		ig::Line<> l{a, b}, m{c, d};
		bool crossing;
		if (a == b) crossing = c == d ? a == c : turn(c, d, a) == 0;
		else if (c == d) crossing = turn(a, b, c) == 0;
		else crossing = (Big(b.x) - a.x) * (Big(d.y) - c.y) != (Big(b.y) - a.y) * (Big(d.x) - c.x) || ac == 0;
		assert(ig::intersects(l, m) == crossing);
		bool line_segment = a == b ? between(c, d, a) : ac * ad <= 0;
		assert(ig::intersects(l, t) == line_segment && ig::intersects(t, l) == line_segment);
	}
	ig::Line<> diagonal{Point(0, 0), Point(3, 3)};
	assert(ig::on_line(diagonal, Point(1, 1)) && !ig::on_line(diagonal, Point(1, 2)));
	assert(ig::on_segment(ig::Segment<>{Point(0, 0), Point(3, 3)}, Point(1, 1)));
	assert(!ig::on_segment(ig::Segment<>{Point(0, 0), Point(3, 3)}, Point(4, 4)));
	assert(ig::parallel(diagonal, ig::Line<>{Point(1, 0), Point(4, 3)}));
	assert(ig::orthogonal(diagonal, ig::Line<>{Point(0, 0), Point(-1, 1)}));
	ig::Line<int> vertical{{1, 0}, {1, 1}};
	assert(ig::intersects(vertical, ig::Segment<>{Point(0, 0), Point(2, 2)}));
	assert(ig::on_line(vertical, Point(1, 5)));
	std::vector<Point> polygon{{0, 0}, {4, 0}, {4, 1}, {1, 1}, {1, 4}, {0, 4}};
	for (int x = -1; x <= 5; ++x) for (int y = -1; y <= 5; ++y) assert(ig::contains(polygon, Point(x, y)) == inside(polygon, Point(x, y)));
	assert(!ig::is_convex(polygon));
	assert(ig::is_convex(std::vector<Point>{{0, 0}, {3, 0}, {3, 3}, {0, 3}}, true));
	assert(!ig::is_convex(std::vector<Point>{{0, 0}, {1, 0}, {3, 0}, {3, 3}, {0, 3}}, true));
}

bool circle_line_oracle(const ig::Circle &circle, const ig::Line<> &line, bool segment) {
	Big ax = Big(line.a.x) - circle.center.x, ay = Big(line.a.y) - circle.center.y;
	Big dx = Big(line.b.x) - line.a.x, dy = Big(line.b.y) - line.a.y;
	Big aa = dx * dx + dy * dy, bb = ax * dx + ay * dy;
	Big cc = ax * ax + ay * ay - Big(circle.radius) * circle.radius;
	if (aa == 0) return cc == 0;
	if (!segment) return bb * bb >= aa * cc;
	Big end = aa + 2 * bb + cc;
	if (cc == 0 || end == 0 || (cc < 0) != (end < 0)) return true;
	if (cc < 0) return false;
	return bb < 0 && -bb < aa && bb * bb >= aa * cc;
}

void circles() {
	std::mt19937 random(13014987);
	for (int trial = 0; trial < 2000; ++trial) {
		Point a{int(random() % 15) - 7, int(random() % 15) - 7};
		Point b{int(random() % 15) - 7, int(random() % 15) - 7};
		Integer ra = random() % 9, rb = random() % 9;
		ig::Circle first{a, ra}, second{b, rb};
		Big d = square(a, b), sum = Big(ra) + rb, difference = Big(ra) - rb;
		ig::CircleRelation expected;
		if (a == b && ra == rb) expected = ig::CircleRelation::coincident;
		else if (d > sum * sum) expected = ig::CircleRelation::separate;
		else if (d == sum * sum) expected = ig::CircleRelation::external_tangent;
		else if (d < difference * difference) expected = ig::CircleRelation::contained;
		else if (d == difference * difference) expected = ig::CircleRelation::internal_tangent;
		else expected = ig::CircleRelation::intersecting;
		assert(ig::relation(first, second) == expected);
		assert(ig::intersects(first, second) == (expected != ig::CircleRelation::contained && expected != ig::CircleRelation::separate));
		Location position = d < Big(ra) * ra ? Location::inside : d == Big(ra) * ra ? Location::boundary : Location::outside;
		assert(ig::contains(first, b) == position);
		Point p{int(random() % 21) - 10, int(random() % 21) - 10};
		Point q{int(random() % 21) - 10, int(random() % 21) - 10};
		ig::Line<> line{p, q};
		ig::Segment<> segment{p, q};
		assert(ig::intersects(first, line) == circle_line_oracle(first, line, false));
		assert(ig::intersects(line, first) == circle_line_oracle(first, line, false));
		assert(ig::intersects(first, segment) == circle_line_oracle(first, line, true));
		assert(ig::intersects(segment, first) == circle_line_oracle(first, line, true));
	}
	constexpr Integer scale = 1000000000;
	ig::Circle circle{Point(-scale, -scale), scale};
	for (auto p : {Point(scale, scale), Point(0, -scale), Point(-scale, 0), Point(-scale, -scale)}) {
		Big d = square(circle.center, p);
		assert(ig::contains(circle, p) == (d > Big(scale) * scale ? Location::outside : d < Big(scale) * scale ? Location::inside : Location::boundary));
	}
	for (auto line : {ig::Line<>{Point(-scale, scale), Point(scale, -scale)}, ig::Line<>{Point(-scale, 0), Point(scale, 0)}, ig::Line<>{Point(-scale, -scale), Point(scale, scale)}}) {
		assert(ig::intersects(circle, line) == circle_line_oracle(circle, line, false));
		assert(ig::intersects(circle, ig::Segment<>{line.a, line.b}) == circle_line_oracle(circle, line, true));
	}
	assert(ig::make_circle(Point(1, 2), Integer(3)).radius == 3);
}

int main() {
	point_and_polar();
	convex_and_lattice();
	linear_and_polygon();
	circles();
}
