#include "integer_geometry/geometry.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

namespace ig = integer_geometry;
using ig::Integer;
using ig::Rational;
using ig::Point;
using ig::RPoint;
using ig::Location;

template<class P>
auto determinant(const P &a, const P &b, const P &c) {
	typename std::remove_cvref_t<decltype(a.x)> result = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	return result;
}

template<class P>
auto square(const P &a, const P &b) {
	typename std::remove_cvref_t<decltype(a.x)> result = (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
	return result;
}

template<class P>
bool between(const P &a, const P &b, const P &p) {
	return determinant(a, b, p) == 0 && std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x)
		&& std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
}

template<class P>
Location inside(const std::vector<P> &polygon, const P &p) {
	int winding = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		const auto &a = polygon[i], &b = polygon[(i + 1) % polygon.size()];
		if (between(a, b, p)) return Location::boundary;
		auto turn = determinant(a, b, p);
		if (a.y <= p.y && p.y < b.y && turn > 0) ++winding;
		if (b.y <= p.y && p.y < a.y && turn < 0) --winding;
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
			auto turn = determinant(points[current], points[next], points[i]);
			if (turn < 0 || (turn == 0 && square(points[current], points[i]) > square(points[current], points[next]))) next = i;
		}
		current = next;
		assert(result.size() <= points.size());
	} while (current != 0);
	return result;
}

void point_and_polar() {
	static_assert(ig::Coordinate<Integer> && ig::Coordinate<Rational>);
	static_assert(!ig::Coordinate<int> && !ig::Coordinate<double>);
	static_assert(!std::is_constructible_v<Point, double, double> && !std::is_constructible_v<RPoint, double, double>);
	static_assert(std::is_same_v<decltype(Point{} / Integer(2)), RPoint>);
	static_assert(std::is_same_v<decltype(Point{} / Rational(2)), RPoint>);
	static_assert(std::is_same_v<decltype(RPoint{} / Rational(2)), RPoint>);
	Integer low = std::numeric_limits<std::int64_t>::min(), high = std::numeric_limits<std::int64_t>::max();
	Point a{low, low}, b{high, low}, c{low, high};
	Integer width = high - low, expected = width * width;
	assert(ig::cross(a, b, c) == expected && ig::orientation(a, b, c) == 1);
	assert(ig::distance_squared(b, c) == 2 * expected);
	assert(ig::dot(a, a) == 2 * low * low && ig::norm2(a) == ig::dot(a, a));
	Integer huge = Integer(1) << 140;
	Point p{huge + 1, -huge - 3};
	assert(-(-p) == p && p + Point(1, 2) - Point(1, 2) == p);
	assert(p * Integer(3) == Integer(3) * p);
	assert(ig::rotate90(p) == Point(huge + 3, huge + 1));
	auto half = Point(1, 3) / Integer(2);
	assert(half == RPoint(Rational(1) / 2, Rational(3) / 2));
	assert(Point(1, 3) / (Rational(2) / 3) == RPoint(Rational(3) / 2, Rational(9) / 2));
	assert(half / (Rational(3) / 2) == RPoint(Rational(1) / 3, 1));
	Point scaled{2, 3};
	scaled *= scaled.x;
	assert(scaled == Point(4, 6));
	RPoint normalized{Rational(2) / 3, Rational(4) / 3};
	normalized /= normalized.x;
	assert(normalized == RPoint(1, 2));
	assert(ig::to_rational(p).x == Rational(huge) + 1);
	assert(ig::ccw(Point(0, 0), Point(2, 0), Point(0, 1)) == 1);
	assert(ig::ccw(Point(0, 0), Point(2, 0), Point(0, -1)) == -1);
	assert(ig::ccw(Point(0, 0), Point(2, 0), Point(-1, 0)) == 2);
	assert(ig::ccw(Point(0, 0), Point(2, 0), Point(3, 0)) == -2);
	assert(ig::ccw(Point(0, 0), Point(2, 0), Point(1, 0)) == 0);
	assert(ig::sign(Integer(-huge)) == -1 && ig::sign(Integer(0)) == 0 && ig::sign(Rational(1) / 3) == 1);
	std::vector<Point> vectors;
	for (int x = -3; x <= 3; ++x) for (int y = -3; y <= 3; ++y) vectors.emplace_back(x, y);
	vectors.insert(vectors.end(), {Point(huge, 1), Point(huge - 1, 1), Point(huge, huge - 1), Point(huge - 1, huge), Point(0, 0)});
	auto oracle = [](const auto &u, const auto &v) {
		bool zero_u = u.x == 0 && u.y == 0, zero_v = v.x == 0 && v.y == 0;
		if (zero_u || zero_v) return zero_u && !zero_v;
		bool back_u = u.y < 0 || (u.y == 0 && u.x < 0), back_v = v.y < 0 || (v.y == 0 && v.x < 0);
		if (back_u != back_v) return back_u < back_v;
		typename std::remove_cvref_t<decltype(u.x)> cross = u.x * v.y - u.y * v.x;
		if (cross != 0) return cross > 0;
		return square(u, std::remove_cvref_t<decltype(u)>{}) < square(v, std::remove_cvref_t<decltype(v)>{});
	};
	auto expected_order = vectors;
	std::sort(expected_order.begin(), expected_order.end(), oracle);
	ig::polar_sort(vectors);
	assert(vectors == expected_order);
	Point center{huge * 2, -huge * 3};
	for (auto &v : vectors) v += center;
	std::reverse(vectors.begin(), vectors.end());
	ig::polar_sort(vectors, center);
	for (auto &v : expected_order) v += center;
	assert(vectors == expected_order);
	std::reverse(vectors.begin(), vectors.end());
	std::sort(vectors.begin(), vectors.end(), ig::PolarLess<Integer>{center});
	assert(vectors == expected_order);
	std::vector<RPoint> rational{{0, 0}, {Rational(1) / 3, 0}, {Rational(1) / 4, 0}, {-1, -1}, {0, Rational(1) / 7}};
	auto sorted = rational;
	std::sort(sorted.begin(), sorted.end(), oracle);
	ig::polar_sort(rational);
	assert(rational == sorted);
	std::vector<Point> empty;
	ig::polar_sort(empty);
	assert(empty.empty());
}

void check_pairs_and_polygon(const std::vector<Point> &points) {
	auto hull = ig::convex_hull(points);
	assert(hull == gift_wrap(points));
	auto indices = ig::convex_hull_indices(points);
	assert(indices.size() == hull.size());
	for (std::size_t i = 0; i < indices.size(); ++i) assert(0 <= indices[i] && indices[i] < int(points.size()) && points[indices[i]] == hull[i]);
	auto nearest = ig::closest_pair(points), furthest = ig::furthest_pair(points);
	if (points.size() < 2) assert(!nearest && !furthest);
	else {
		Integer minimum = square(points[0], points[1]), maximum = minimum;
		for (std::size_t i = 0; i < points.size(); ++i) for (std::size_t j = 0; j < i; ++j) {
			Integer d = square(points[i], points[j]);
			minimum = std::min(minimum, d); maximum = std::max(maximum, d);
		}
		for (auto result : {nearest, furthest}) {
			assert(result && 0 <= result->first && result->first < int(points.size()));
			assert(0 <= result->second && result->second < int(points.size()) && result->first != result->second);
			assert(result->squared_distance == square(points[result->first], points[result->second]));
		}
		assert(nearest->squared_distance == minimum && furthest->squared_distance == maximum);
		assert(ig::diameter_squared(hull) == maximum);
	}
	auto diameter = ig::convex_diameter(hull);
	assert(bool(diameter) == !hull.empty());
	if (diameter) assert(diameter->squared_distance == square(hull[diameter->first], hull[diameter->second]));
	Integer twice = 0;
	for (std::size_t i = 0; i < hull.size(); ++i) twice += hull[i].x * hull[(i + 1) % hull.size()].y - hull[i].y * hull[(i + 1) % hull.size()].x;
	assert(ig::signed_area2(hull) == twice && ig::area(hull) == Rational(twice) / 2);
	for (int x = -6; x <= 6; ++x) for (int y = -6; y <= 6; ++y) {
		Point p{x, y};
		assert(ig::convex_contains(hull, p) == inside(hull, p));
		assert(ig::contains(hull, p) == inside(hull, p));
	}
	if (hull.size() >= 3) {
		Integer count = 0, boundary = 0;
		for (int x = -6; x <= 6; ++x) for (int y = -6; y <= 6; ++y) {
			auto where = inside(hull, Point{x, y});
			count += where == Location::inside;
			boundary += where == Location::boundary;
		}
		assert(ig::boundary_lattice_points(hull) == boundary);
		assert(ig::interior_lattice_points(hull) == count);
	} else assert(ig::interior_lattice_points(hull) == 0);
}

void convex_and_lattice() {
	std::mt19937 random(89364013);
	for (int trial = 0; trial < 100; ++trial) {
		std::vector<Point> points;
		int n = random() % 24;
		for (int i = 0; i < n; ++i) points.emplace_back(int(random() % 11) - 5, int(random() % 11) - 5);
		check_pairs_and_polygon(points);
		std::vector<Point> other;
		for (int i = 0; i < 6; ++i) other.emplace_back(int(random() % 11) - 5, int(random() % 11) - 5);
		std::vector<Point> sums;
		for (const auto &p : points) for (const auto &q : other) sums.push_back(p + q);
		assert(ig::minkowski_sum(ig::convex_hull(points), ig::convex_hull(other)) == gift_wrap(sums));
	}
	Integer huge = Integer(1) << 150;
	std::vector<Point> points{{huge, huge}, {huge + 3, huge}, {huge, huge + 4}, {huge + 1, huge + 1}, {huge, huge}};
	auto nearest = ig::closest_pair(points), furthest = ig::furthest_pair(points);
	assert(nearest && nearest->squared_distance == 0 && nearest->first != nearest->second);
	assert(furthest && furthest->squared_distance == 25);
	auto hull = ig::convex_hull(points);
	assert(ig::area(hull) == 6 && ig::boundary_lattice_points(hull) == 8);
	assert(ig::interior_lattice_points(hull) == 3);
	auto center = ig::centroid(hull);
	assert(center && *center == RPoint(Rational(huge) + 1, Rational(huge) + Rational(4) / 3));
	assert(!ig::centroid(std::vector<Point>{{0, 0}, {1, 1}, {2, 2}}));
	std::vector<Point> border{{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}, {0, 1}, {1, 1}, {0, 0}};
	auto all = ig::convex_hull(border, true);
	assert((all == std::vector<Point>{{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}, {0, 1}}));
	assert(ig::gcd(Integer(-huge * 6), Integer(huge * 9)) == huge * 3);
	assert(ig::primitive_direction(Point(huge * 6, -huge * 9)) == Point(2, -3));
	assert(ig::primitive_direction(Point{}) == Point{});
	for (int x = -5; x <= 5; ++x) for (int y = -5; y <= 5; ++y) {
		assert(ig::segment_lattice_points(Point(-2, 3), Point(x, y)) == std::gcd(x + 2, y - 3) + 1);
	}
	assert(ig::gcd(Integer(0), Integer(0)) == 0 && ig::gcd(Integer(-7), Integer(0)) == 7);
	assert(ig::boundary_lattice_points({}) == 0 && ig::interior_lattice_points({}) == 0);
	assert(ig::boundary_lattice_points({Point(1, 2)}) == 1);
	assert(ig::boundary_lattice_points({Point(-2, -2), Point(3, 3), Point(0, 0), Point(-2, -2)}) == 6);
	assert(ig::interior_lattice_points({Point(-2, -2), Point(3, 3), Point(0, 0), Point(-2, -2)}) == 0);
	border = {{0, 0}, {2, 0}, {2, 2}, {0, 2}, {0, 0}};
	assert(ig::boundary_lattice_points(border) == 8);
	assert(ig::interior_lattice_points(border) == 1);
	std::reverse(border.begin(), border.end());
	assert(ig::signed_area2(border) == -8 && ig::area(border) == 4 && ig::interior_lattice_points(border) == 1);
	std::vector<Point> integer_points{{0, 0}, {3, 0}, {0, 4}, {1, 1}, {3, 0}};
	std::vector<RPoint> fractions;
	for (const auto &p : integer_points) fractions.push_back(p / Integer(7));
	auto fractional_hull = ig::convex_hull(fractions);
	auto integer_hull = gift_wrap(integer_points);
	assert(fractional_hull.size() == integer_hull.size());
	for (std::size_t i = 0; i < integer_hull.size(); ++i) assert(fractional_hull[i] == integer_hull[i] / Integer(7));
	auto fractional_near = ig::closest_pair(fractions), fractional_far = ig::furthest_pair(fractions);
	assert(fractional_near && fractional_near->squared_distance == 0);
	assert(fractional_far && fractional_far->squared_distance == Rational(25) / 49);
	assert(ig::diameter_squared(fractional_hull) == Rational(25) / 49);
	assert(ig::convex_contains(fractional_hull, RPoint(Rational(1) / 7, Rational(1) / 7)) == Location::inside);
}

void linear_and_cut() {
	std::vector<Point> points{{-1, -1}, {1, -1}, {1, 1}, {-1, 1}, {0, 0}};
	for (const auto &a : points) for (const auto &b : points) for (const auto &c : points) for (const auto &d : points) {
		auto direction = [](const Point &x, const Point &y, const Point &z) { auto v = determinant(x, y, z); return (v > 0) - (v < 0); };
		int ac = direction(a, b, c), ad = direction(a, b, d), ca = direction(c, d, a), cb = direction(c, d, b);
		bool expected = between(a, b, c) || between(a, b, d) || between(c, d, a) || between(c, d, b)
			|| (ac * ad < 0 && ca * cb < 0);
		ig::Segment<> s{a, b}, t{c, d};
		assert(ig::intersects(s, t) == expected);
		auto intersections = ig::intersection(s, t);
		assert((!intersections.empty()) == expected && intersections.size() <= 2);
		for (const auto &p : intersections) {
			assert(between(ig::to_rational(a), ig::to_rational(b), p));
			assert(between(ig::to_rational(c), ig::to_rational(d), p));
		}
		if (intersections.size() == 2) assert(intersections[0] != intersections[1]);
	}
	ig::Line<> diagonal{Point(0, 0), Point(3, 3)}, second{Point(0, 2), Point(3, 0)};
	auto crossing = ig::intersection(diagonal, second);
	assert(crossing && *crossing == RPoint(Rational(6) / 5, Rational(6) / 5));
	assert(ig::projection(diagonal, Point(1, 2)) == RPoint(Rational(3) / 2, Rational(3) / 2));
	assert(ig::reflection(diagonal, Point(1, 2)) == RPoint(2, 1));
	assert(ig::distance_squared(diagonal, Point(1, 2)) == Rational(1) / 2);
	assert(ig::distance_squared(Point(1, 2), diagonal) == Rational(1) / 2);
	assert(ig::distance_squared(ig::Segment<>{Point(0, 0), Point(1, 0)}, Point(2, 1)) == 2);
	assert(ig::distance_squared(ig::Segment<>{Point(0, 0), Point(0, 0)}, Point(2, 1)) == 5);
	assert(!ig::perpendicular_bisector(Point(1, 1), Point(1, 1)));
	auto bisector = ig::perpendicular_bisector(Point(0, 0), Point(1, 0));
	assert(bisector && ig::on_line(*bisector, RPoint(Rational(1) / 2, 9)));
	assert(ig::parallel(diagonal, ig::Line<>{Point(1, 0), Point(4, 3)}));
	assert(ig::orthogonal(diagonal, ig::Line<>{Point(0, 0), Point(-1, 1)}));
	ig::Line<Rational> vertical{RPoint(Rational(1) / 2, 0), RPoint(Rational(1) / 2, 1)};
	auto mixed = ig::intersection(diagonal, vertical);
	assert(mixed && *mixed == RPoint(Rational(1) / 2, Rational(1) / 2));
	assert(ig::distance_squared(vertical, Point(2, 0)) == Rational(9) / 4);
	assert(ig::intersects(vertical, ig::Segment<>{Point(0, 0), Point(1, 1)}));
	std::vector<Point> square_polygon{{0, 0}, {3, 0}, {3, 3}, {0, 3}};
	auto clipped = ig::convex_cut(square_polygon, ig::Line<>{Point(0, 1), Point(2, 0)});
	assert(ig::area(clipped) == 8);
	for (const auto &p : clipped) assert(p.x + 2 * p.y >= 2);
	assert(ig::contains(square_polygon, RPoint(Rational(1) / 2, Rational(1) / 2)) == Location::inside);
	assert(ig::contains(clipped, RPoint(Rational(1) / 2, Rational(1) / 2)) == Location::outside);
	assert(ig::is_convex(square_polygon));
	assert(!ig::is_convex(std::vector<Point>{{0, 0}, {3, 0}, {1, 1}, {0, 3}}));
}

struct CircleOracle { RPoint center; Rational radius_squared; };

std::optional<CircleOracle> enclosing_oracle(const std::vector<Point> &points) {
	std::optional<CircleOracle> best;
	auto consider = [&](RPoint center, Rational radius_squared) {
		for (const auto &p : points) if (square(center, ig::to_rational(p)) > radius_squared) return;
		if (!best || radius_squared < best->radius_squared) best = CircleOracle{std::move(center), std::move(radius_squared)};
	};
	for (const auto &p : points) consider(ig::to_rational(p), 0);
	for (std::size_t i = 0; i < points.size(); ++i) for (std::size_t j = 0; j < i; ++j) {
		RPoint center((Rational(points[i].x) + Rational(points[j].x)) / 2, (Rational(points[i].y) + Rational(points[j].y)) / 2);
		consider(center, square(center, ig::to_rational(points[i])));
		for (std::size_t k = 0; k < j; ++k) {
			const auto &a = points[i], &b = points[j], &c = points[k];
			Integer d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
			if (d == 0) continue;
			Integer aa = a.x * a.x + a.y * a.y, bb = b.x * b.x + b.y * b.y, cc = c.x * c.x + c.y * c.y;
			Rational denominator = d;
			RPoint center(Rational(aa * (b.y - c.y) + bb * (c.y - a.y) + cc * (a.y - b.y)) / denominator,
				Rational(aa * (c.x - b.x) + bb * (a.x - c.x) + cc * (b.x - a.x)) / denominator);
			consider(center, square(center, ig::to_rational(a)));
		}
	}
	return best;
}

void circles() {
	auto circle = ig::make_circle(Point(0, 0), Rational(3));
	assert(circle.radius_squared == 9 && circle.center == RPoint{});
	assert(ig::contains(circle, Point(0, 0)) == Location::inside);
	assert(ig::contains(circle, Point(3, 0)) == Location::boundary);
	assert(ig::contains(circle, Point(3, 1)) == Location::outside);
	assert(ig::relation(circle, ig::make_circle(Point(6, 0), Rational(3))) == ig::CircleRelation::external_tangent);
	assert(ig::relation(circle, ig::make_circle(Point(0, 0), Rational(3))) == ig::CircleRelation::coincident);
	assert(ig::intersects(circle, ig::Line<>{Point(-1, 3), Point(1, 3)}));
	assert(!ig::intersects(circle, ig::Segment<>{Point(0, 0), Point(1, 1)}));
	auto circum = ig::circumcircle(Point(0, 0), Point(3, 0), Point(0, 4));
	assert(circum && circum->center == RPoint(Rational(3) / 2, 2) && circum->radius_squared == Rational(25) / 4);
	assert(!ig::circumcircle(Point(0, 0), Point(1, 1), Point(2, 2)));
	std::mt19937 random(2485472);
	for (int trial = 0; trial < 80; ++trial) {
		std::vector<Point> points;
		for (unsigned i = 0, n = random() % 8; i < n; ++i) points.emplace_back(int(random() % 11) - 5, int(random() % 11) - 5);
		auto expected = enclosing_oracle(points);
		auto actual = ig::minimum_enclosing_circle(points, random());
		assert(bool(actual) == bool(expected));
		if (actual) {
			assert(actual->radius_squared == expected->radius_squared);
			for (const auto &p : points) assert(ig::contains(*actual, p) != Location::outside);
		}
	}
	Integer huge = Integer(1) << 100;
	std::vector<Point> points{{huge, huge}, {huge + 3, huge}, {huge, huge + 4}};
	auto big = ig::minimum_enclosing_circle(points);
	assert(big && big->center == RPoint(Rational(huge) + Rational(3) / 2, Rational(huge) + 2));
	assert(big->radius_squared == Rational(25) / 4);
}

int main() {
	point_and_polar();
	convex_and_lattice();
	linear_and_cut();
	circles();
}
