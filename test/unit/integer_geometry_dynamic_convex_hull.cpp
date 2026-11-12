#include "integer_geometry/dynamic_convex_hull.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

using integer_geometry::DynamicConvexHull;
using integer_geometry::Location;
using integer_geometry::Point;
using integer_geometry::Wide;

Wide turn(Point a, Point b, Point c) {
	return (Wide(b.x) - a.x) * (Wide(c.y) - a.y) - (Wide(b.y) - a.y) * (Wide(c.x) - a.x);
}

Wide squared(Point a, Point b) {
	Wide x = Wide(a.x) - b.x, y = Wide(a.y) - b.y;
	return x * x + y * y;
}

std::vector<Point> wrap(std::vector<Point> points) {
	std::sort(points.begin(), points.end());
	points.erase(std::unique(points.begin(), points.end()), points.end());
	if (points.size() < 3) return points;
	std::vector<Point> result;
	std::size_t current = 0;
	do {
		result.push_back(points[current]);
		std::size_t next = current == 0 ? 1 : 0;
		for (std::size_t i = 0; i < points.size(); ++i) if (i != current) {
			Wide value = turn(points[current], points[next], points[i]);
			if (value < 0 || (value == 0 && squared(points[current], points[i]) > squared(points[current], points[next]))) next = i;
		}
		current = next;
		assert(result.size() <= points.size());
	} while (current != 0);
	return result;
}

Location locate(const std::vector<Point> &polygon, Point p) {
	int winding = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		auto a = polygon[i], b = polygon[(i + 1) % polygon.size()];
		Wide value = turn(a, b, p);
		if (!value && std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x)
			&& std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y)) return Location::boundary;
		if (a.y <= p.y && p.y < b.y && value > 0) ++winding;
		if (b.y <= p.y && p.y < a.y && value < 0) --winding;
	}
	return winding ? Location::inside : Location::outside;
}

void check(const DynamicConvexHull<> &hull, const std::vector<Point> &points, std::mt19937_64 &random) {
	auto expected = wrap(points);
	assert(hull.vertices() == expected && hull.size() == expected.size() && hull.empty() == expected.empty());
	Wide area = 0;
	for (std::size_t i = 0; i < expected.size(); ++i) {
		auto a = expected[i], b = expected[(i + 1) % expected.size()];
		area += Wide(a.x) * b.y - Wide(a.y) * b.x;
	}
	assert(hull.doubled_area() == area);
	for (int i = 0; i < 12; ++i) {
		Point p{std::int64_t(random() % 61) - 30, std::int64_t(random() % 61) - 30};
		assert(hull.contains(p) == locate(expected, p));
		if (points.empty()) {
			assert(!hull.support(p) && !hull.max_dot(p) && !hull.min_dot(p));
			continue;
		}
		Wide minimum = Wide(points[0].x) * p.x + Wide(points[0].y) * p.y, maximum = minimum;
		for (auto q : points) {
			Wide value = Wide(q.x) * p.x + Wide(q.y) * p.y;
			minimum = std::min(minimum, value); maximum = std::max(maximum, value);
		}
		assert(hull.min_dot(p) == minimum && hull.max_dot(p) == maximum);
		auto support = hull.support(p);
		assert(support && Wide(support->x) * p.x + Wide(support->y) * p.y == maximum);
	}
}

int main() {
	std::mt19937_64 random(943922);
	for (int trial = 0; trial < 40; ++trial) {
		DynamicConvexHull<> hull;
		std::vector<Point> points;
		check(hull, points, random);
		for (int i = 0; i < 55; ++i) {
			Point p{std::int64_t(random() % 41) - 20, std::int64_t(random() % 41) - 20};
			if (trial % 5 == 0) p.y = p.x;
			if (trial % 5 == 1) p.x = 0;
			if (trial % 5 == 2) p = {1, 2};
			auto before = wrap(points);
			points.push_back(p);
			assert(hull.insert(p) == (before != wrap(points)));
			check(hull, points, random);
		}
		auto copy = hull;
		hull.clear();
		assert(hull.empty());
		check(copy, points, random);
		auto moved = std::move(copy);
		assert(copy.empty());
		copy.insert({7, 8});
		check(copy, {{7, 8}}, random);
		check(moved, points, random);
		auto *self = &moved;
		moved = *self;
		check(moved, points, random);
		copy = std::move(moved);
		assert(moved.empty());
		check(copy, points, random);
		swap(copy, hull);
		assert(copy.empty());
		check(hull, points, random);
	}
	constexpr std::int64_t bound = 1000000000000000000LL;
	DynamicConvexHull<> large({{-bound, -bound}, {bound, -bound}, {bound, bound}, {-bound, bound}});
	assert(large.doubled_area() == Wide(8) * bound * bound);
	assert(large.max_dot({bound, bound}) == Wide(2) * bound * bound);
	assert(large.min_dot({bound, bound}) == -Wide(2) * bound * bound);
	assert(large.contains({0, 0}) == Location::inside);
	assert(large.support({0, 0}));
	assert(!large.insert({0, 0}));
	DynamicConvexHull<std::int32_t> narrow({{INT32_MIN, INT32_MIN}, {INT32_MAX, INT32_MAX}});
	assert(narrow.min_dot({INT32_MIN, 0}) == Wide(INT32_MIN) * INT32_MAX);
}
