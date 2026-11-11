#include "geometry/exact_circle.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <vector>

using namespace geometry;
using I = std::int64_t;

I square(IPoint a, IPoint b) {
	I x = a.x - b.x, y = a.y - b.y;
	return x * x + y * y;
}

CircleRelation reference(IPoint a, I r, IPoint b, I s) {
	I d = square(a, b);
	if (!d && r == s) return CircleRelation::coincident;
	if (d > (r + s) * (r + s)) return CircleRelation::separate;
	if (d < (r - s) * (r - s)) return CircleRelation::contained;
	if (d == (r + s) * (r + s)) return CircleRelation::external_tangent;
	if (d == (r - s) * (r - s)) return CircleRelation::internal_tangent;
	return CircleRelation::intersecting;
}

bool reference_segment(IPoint a, IPoint b, I r) {
	I n = square(a, b), radius2 = r * r;
	if (!n) return square({}, a) == radius2;
	I d = -a.x * (b.x - a.x) - a.y * (b.y - a.y);
	bool close;
	if (d <= 0) close = square({}, a) <= radius2;
	else if (d >= n) close = square({}, b) <= radius2;
	else {
		I determinant = a.x * b.y - a.y * b.x;
		close = determinant * determinant <= radius2 * n;
	}
	return close && std::max(square({}, a), square({}, b)) >= radius2;
}

std::optional<ExactCircle> reference_minimum(const std::vector<RPoint> &points) {
	std::optional<ExactCircle> best;
	auto consider = [&](RPoint center, Rational radius2) {
		if (best && best->radius_squared <= radius2) return;
		for (const auto &point : points) {
			Rational x = point.x - center.x, y = point.y - center.y;
			if (x * x + y * y > radius2) return;
		}
		best = ExactCircle{center, radius2};
	};
	for (const auto &point : points) consider(point, 0);
	for (std::size_t i = 0; i < points.size(); ++i) for (std::size_t j = 0; j < i; ++j) {
		RPoint center{Rational((points[i].x + points[j].x) / 2), Rational((points[i].y + points[j].y) / 2)};
		Rational x = points[i].x - center.x, y = points[i].y - center.y;
		consider(center, Rational(x * x + y * y));
		for (std::size_t k = 0; k < j; ++k) {
			const auto &a = points[i], &b = points[j], &c = points[k];
			Rational bx = 2 * (b.x - a.x), by = 2 * (b.y - a.y);
			Rational cx = 2 * (c.x - a.x), cy = 2 * (c.y - a.y);
			Rational det = bx * cy - by * cx;
			if (det == 0) continue;
			Rational bn = b.x * b.x + b.y * b.y - a.x * a.x - a.y * a.y;
			Rational cn = c.x * c.x + c.y * c.y - a.x * a.x - a.y * a.y;
			RPoint circumcenter{Rational((bn * cy - by * cn) / det), Rational((bx * cn - bn * cx) / det)};
			Rational dx = a.x - circumcenter.x, dy = a.y - circumcenter.y;
			consider(circumcenter, Rational(dx * dx + dy * dy));
		}
	}
	return best;
}

template<ExactCoordinate T>
void check_minimum(const std::vector<Point<T>> &points) {
	std::vector<RPoint> converted;
	for (const auto &p : points) converted.push_back({Rational(p.x), Rational(p.y)});
	auto expected = reference_minimum(converted);
	for (std::uint64_t seed : std::array<std::uint64_t, 5>{0, 1, 7, 712367821, UINT64_MAX}) {
		auto actual = minimum_enclosing_circle_exact(points, seed);
		assert(bool(actual) == bool(expected));
		if (!actual) continue;
		assert(actual->center == expected->center && actual->radius_squared == expected->radius_squared);
		for (const auto &p : converted) {
			Rational x = p.x - actual->center.x, y = p.y - actual->center.y;
			assert(x * x + y * y <= actual->radius_squared);
		}
	}
	auto default_seed = minimum_enclosing_circle_exact(points);
	assert(bool(default_seed) == bool(expected));
	if (default_seed) assert(default_seed->center == expected->center && default_seed->radius_squared == expected->radius_squared);
}

void check_minimum_enclosing_circle() {
	std::array<IPoint, 9> grid{{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 0}, {0, 1}, {1, -1}, {1, 0}, {1, 1}}};
	for (unsigned mask = 0; mask < (1U << grid.size()); ++mask) {
		std::vector<IPoint> points;
		for (std::size_t i = 0; i < grid.size(); ++i) if (mask >> i & 1) points.push_back(grid[i]);
		check_minimum(points);
	}
	std::mt19937_64 random(7193);
	for (int trial = 0; trial < 100; ++trial) {
		std::vector<IPoint> points;
		for (unsigned i = 0, n = random() % 9; i < n; ++i)
			points.push_back({I(random() % 17) - 8, I(random() % 17) - 8});
		check_minimum(points);
	}
	std::vector<IPoint> collinear;
	for (I x = -6; x <= 6; ++x) collinear.push_back({x, 3 * x + 1});
	collinear.push_back(collinear.front());
	check_minimum(collinear);
	check_minimum(std::vector<IPoint>(12, IPoint{4, -5}));
	check_minimum(std::vector<IPoint>{{0, 0}, {4, 0}, {2, 3}, {2, 1}});
	check_minimum(std::vector<IPoint>{{-2, 0}, {2, 0}, {0, 1}});
	Integer huge = Integer(1) << 150;
	check_minimum(std::vector<BigPoint>{{huge, -huge}, {huge + 4, -huge}, {huge, -huge + 4}, {huge + 1, -huge + 1}});
	check_minimum(std::vector<RPoint>{{Rational(1) / 3, Rational(2) / 5}, {Rational(4) / 3, Rational(2) / 5},
		{Rational(1) / 3, Rational(7) / 5}, {Rational(1) / 2, Rational(1) / 2}});
	constexpr I low = std::numeric_limits<I>::min(), high = std::numeric_limits<I>::max();
	check_minimum(std::vector<IPoint>{{low, low}, {high, low}, {high, high}, {low, high}});
}

int main() {
	check_minimum_enclosing_circle();
	std::vector<IPoint> points;
	for (I x = -2; x <= 2; ++x) for (I y = -2; y <= 2; ++y) points.push_back({x, y});
	for (IPoint a : points) for (I r = 0; r <= 3; ++r) {
		auto first = make_exact_circle(a, r);
		for (IPoint b : points) {
			I d = square(a, b), rr = r * r;
			auto expected = d < rr ? Location::inside : d == rr ? Location::boundary : Location::outside;
			assert(contains(first, b) == expected);
			for (I s = 0; s <= 3; ++s) {
				auto second = make_exact_circle(b, s);
				auto type = reference(a, r, b, s);
				assert(relation(first, second) == type && relation(second, first) == type);
				assert(intersects(first, second) == (type != CircleRelation::separate && type != CircleRelation::contained));
			}
		}
	}
	for (IPoint a : points) for (IPoint b : points) for (I r = 0; r <= 3; ++r) {
		auto circle = make_exact_circle(IPoint{}, r);
		Line<I> line{a, b};
		Segment<I> segment{a, b};
		I n = square(a, b), determinant = a.x * b.y - a.y * b.x;
		bool expected_line = !n ? square({}, a) == r * r : determinant * determinant <= r * r * n;
		assert(intersects(circle, line) == expected_line && intersects(line, circle) == expected_line);
		bool expected_segment = reference_segment(a, b, r);
		assert(intersects(circle, segment) == expected_segment && intersects(segment, circle) == expected_segment);
	}

	ExactCircle irrational{{0, 0}, 2}, larger{{3, 3}, 8};
	assert(relation(irrational, larger) == CircleRelation::external_tangent);
	larger.center = {1, 1};
	assert(relation(irrational, larger) == CircleRelation::internal_tangent);
	larger.center = {0, 0};
	assert(relation(irrational, larger) == CircleRelation::contained);
	larger.center = {2, 0};
	assert(relation(irrational, larger) == CircleRelation::intersecting);
	larger.center = {4, 4};
	assert(relation(irrational, larger) == CircleRelation::separate);
	Rational tiny = Rational(1) / (Integer(1) << 200);
	larger.center = {3, Rational(3 + tiny)};
	assert(relation(irrational, larger) == CircleRelation::separate);
	larger.center = {3, Rational(3 - tiny)};
	assert(relation(irrational, larger) == CircleRelation::intersecting);
	larger.center = {1, Rational(1 - tiny)};
	assert(relation(irrational, larger) == CircleRelation::contained);
	larger.center = {1, Rational(1 + tiny)};
	assert(relation(irrational, larger) == CircleRelation::intersecting);
	auto fractional = make_exact_circle(RPoint{Rational(1) / 3, Rational(2) / 7}, Rational(3) / 2);
	assert(fractional.radius_squared == Rational(9) / 4);
	assert(contains(fractional, RPoint{Rational(11) / 6, Rational(2) / 7}) == Location::boundary);
	assert(intersects(fractional, Line<Rational>{{Rational(11) / 6, 0}, {Rational(11) / 6, 1}}));
	assert(!intersects(fractional, Line<Rational>{{Rational(11) / 6 + tiny, 0}, {Rational(11) / 6 + tiny, 1}}));

	for (IPoint a : points) for (IPoint b : points) for (IPoint c : points) {
		I determinant = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		auto circle = circumcircle_exact(a, b, c);
		assert(bool(circle) == (determinant != 0));
		if (circle) {
			assert(contains(*circle, a) == Location::boundary);
			assert(contains(*circle, b) == Location::boundary);
			assert(contains(*circle, c) == Location::boundary);
			Rational x = circle->center.x, y = circle->center.y;
			assert(2 * ((b.x - a.x) * x + (b.y - a.y) * y) == square({}, b) - square({}, a));
			assert(2 * ((c.x - a.x) * x + (c.y - a.y) * y) == square({}, c) - square({}, a));
		}
	}
	auto rational_triangle = circumcircle_exact(RPoint{}, RPoint{Rational(1) / 3, 0}, RPoint{0, Rational(1) / 5});
	assert(rational_triangle && (rational_triangle->center == RPoint{Rational(1) / 6, Rational(1) / 10}));
	assert(rational_triangle->radius_squared == Rational(17) / 450);
	Integer huge = Integer(1) << 150;
	auto enormous = make_exact_circle(BigPoint{huge, -huge}, Rational(huge));
	assert(contains(enormous, BigPoint{2 * huge, -huge}) == Location::boundary);
	assert(contains(enormous, BigPoint{2 * huge + 1, -huge}) == Location::outside);
	assert(relation(enormous, make_exact_circle(BigPoint{2 * huge, -huge}, 0)) == CircleRelation::external_tangent);
	auto shifted = circumcircle_exact(BigPoint{huge, -huge}, BigPoint{huge + 2, -huge}, BigPoint{huge, -huge + 2});
	assert(shifted && shifted->center == RPoint(huge + 1, -huge + 1) && shifted->radius_squared == 2);
	constexpr I low = std::numeric_limits<I>::min(), high = std::numeric_limits<I>::max();
	auto extreme = circumcircle_exact(IPoint{low, low}, IPoint{high, low}, IPoint{low, high});
	assert(extreme && (extreme->center == RPoint{Rational(-1) / 2, Rational(-1) / 2}));
	Integer width = Integer(high) - low;
	assert(extreme->radius_squared == Rational(width * width) / 2);
	assert(contains(*extreme, IPoint{high, high}) == Location::boundary);
}
