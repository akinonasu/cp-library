#include "geometry/exact_linear.hpp"
#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

using namespace geometry;

template<class T>
concept ExactProjection = requires(Line<T> line, Point<T> point) { projection_exact(line, point); };
static_assert(ExactProjection<std::int64_t> && ExactProjection<Integer> && ExactProjection<Rational>);
static_assert(!ExactProjection<double> && !ExactProjection<Real>);
static_assert(std::same_as<decltype(distance_squared_exact(IPoint{}, IPoint{})), Rational>);

Rational fraction(std::int64_t numerator, std::int64_t denominator) { return Rational(numerator) / denominator; }

struct Equation {
	std::int64_t a, b, c;
	explicit Equation(Line<std::int64_t> line)
		: a(line.a.y - line.b.y), b(line.b.x - line.a.x), c(line.a.x * line.b.y - line.a.y * line.b.x) {}
	std::int64_t evaluate(IPoint point) const { return a * point.x + b * point.y + c; }
};

bool on_reference(Line<std::int64_t> line, IPoint point) {
	return line.a == line.b ? line.a == point : Equation(line).evaluate(point) == 0;
}

bool on_reference(Segment<std::int64_t> segment, RPoint point) {
	Equation equation({segment.a, segment.b});
	if (equation.a * point.x + equation.b * point.y + equation.c != 0) return false;
	return std::min(segment.a.x, segment.b.x) <= point.x && point.x <= std::max(segment.a.x, segment.b.x)
		&& std::min(segment.a.y, segment.b.y) <= point.y && point.y <= std::max(segment.a.y, segment.b.y);
}

std::optional<RPoint> line_reference(Line<std::int64_t> line, Line<std::int64_t> other) {
	if (line.a == line.b) return on_reference(other, line.a) ? std::optional<RPoint>(to_rational(line.a)) : std::nullopt;
	if (other.a == other.b) return on_reference(line, other.a) ? std::optional<RPoint>(to_rational(other.a)) : std::nullopt;
	Equation first(line), second(other);
	auto determinant = first.a * second.b - second.a * first.b;
	if (!determinant) return std::nullopt;
	return RPoint{fraction(first.b * second.c - second.b * first.c, determinant),
		fraction(first.c * second.a - second.c * first.a, determinant)};
}

std::vector<RPoint> segment_reference(Segment<std::int64_t> segment, Segment<std::int64_t> other) {
	if (auto p = line_reference({segment.a, segment.b}, {other.a, other.b}))
		return on_reference(segment, *p) && on_reference(other, *p) ? std::vector<RPoint>{*p} : std::vector<RPoint>{};
	std::vector<RPoint> candidates;
	for (auto point : {segment.a, segment.b, other.a, other.b}) {
		auto p = to_rational(point);
		if (on_reference(segment, p) && on_reference(other, p)) candidates.push_back(p);
	}
	std::sort(candidates.begin(), candidates.end());
	candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
	if (candidates.size() > 1) return {candidates.front(), candidates.back()};
	return candidates;
}

std::vector<RPoint> mixed_reference(Line<std::int64_t> line, Segment<std::int64_t> segment) {
	if (auto p = line_reference(line, {segment.a, segment.b}))
		return on_reference(segment, *p) ? std::vector<RPoint>{*p} : std::vector<RPoint>{};
	if (on_reference(line, segment.a) && on_reference(line, segment.b)) {
		if (segment.a == segment.b) return {to_rational(segment.a)};
		return {to_rational(segment.a), to_rational(segment.b)};
	}
	return {};
}

RPoint projection_reference(Line<std::int64_t> line, IPoint point) {
	if (line.a == line.b) return to_rational(line.a);
	Equation equation(line);
	Rational ratio = fraction(equation.evaluate(point), equation.a * equation.a + equation.b * equation.b);
	return {Rational(point.x) - equation.a * ratio, Rational(point.y) - equation.b * ratio};
}

Rational point_reference(RPoint a, RPoint b) {
	Rational x = a.x - b.x, y = a.y - b.y;
	return x * x + y * y;
}

Rational distance_reference(Line<std::int64_t> line, IPoint point) {
	return point_reference(projection_reference(line, point), to_rational(point));
}

Rational distance_reference(Segment<std::int64_t> segment, IPoint point) {
	auto projected = projection_reference({segment.a, segment.b}, point);
	if (on_reference(segment, projected)) return point_reference(projected, to_rational(point));
	return std::min(point_reference(to_rational(segment.a), to_rational(point)),
		point_reference(to_rational(segment.b), to_rational(point)));
}

int main() {
	std::vector<IPoint> points;
	for (int x = -1; x <= 1; ++x) for (int y = -1; y <= 1; ++y) points.emplace_back(x, y);
	std::vector<Line<std::int64_t>> lines;
	for (auto a : points) for (auto b : points) lines.push_back({a, b});
	for (auto line : lines) {
		Segment<std::int64_t> segment{line.a, line.b};
		for (auto point : points) {
			auto projection = projection_exact(line, point);
			assert(projection == projection_reference(line, point));
			assert(reflection_exact(line, point) == projection * Rational(2) - to_rational(point));
			assert(distance_squared_exact(line, point) == distance_reference(line, point));
			assert(distance_squared_exact(point, line) == distance_reference(line, point));
			assert(distance_squared_exact(segment, point) == distance_reference(segment, point));
			assert(distance_squared_exact(point, segment) == distance_reference(segment, point));
			assert(distance_squared_exact(line, projection) == 0);
		}
		for (auto other : lines) {
			Segment<std::int64_t> second{other.a, other.b};
			auto line_intersection = line_reference(line, other);
			auto segment_intersection = segment_reference(segment, second);
			auto mixed_intersection = mixed_reference(line, second);
			assert(intersection_exact(line, other) == line_intersection);
			assert(intersection_exact(segment, second) == segment_intersection);
			assert(intersection_exact(line, second) == mixed_intersection);
			assert(intersection_exact(second, line) == mixed_intersection);
			Rational line_distance = 0;
			if (!line_intersection && !on_reference(line, other.a))
				line_distance = line.a == line.b ? distance_reference(other, line.a) : distance_reference(line, other.a);
			Rational mixed_distance = 0;
			if (mixed_intersection.empty()) mixed_distance = line.a == line.b ? distance_reference(second, line.a)
				: std::min(distance_reference(line, second.a), distance_reference(line, second.b));
			Rational segment_distance = 0;
			if (segment_intersection.empty()) segment_distance = std::min({distance_reference(segment, second.a),
				distance_reference(segment, second.b), distance_reference(second, segment.a), distance_reference(second, segment.b)});
			assert(distance_squared_exact(line, other) == line_distance);
			assert(distance_squared_exact(line, second) == mixed_distance);
			assert(distance_squared_exact(second, line) == mixed_distance);
			assert(distance_squared_exact(segment, second) == segment_distance);
		}
	}
	for (auto a : points) for (auto b : points) {
		auto bisector = perpendicular_bisector_exact(a, b);
		if (a == b) assert(!bisector);
		else {
			assert(bisector);
			assert(distance_squared_exact(bisector->a, a) == distance_squared_exact(bisector->a, b));
			assert(distance_squared_exact(bisector->b, a) == distance_squared_exact(bisector->b, b));
		}
	}
	constexpr auto low = std::numeric_limits<std::int64_t>::min(), high = std::numeric_limits<std::int64_t>::max();
	Line<std::int64_t> diagonal{{low, low}, {high, high}}, reverse{{low, high}, {high, low}};
	assert((intersection_exact(diagonal, reverse) == RPoint{fraction(-1, 2), fraction(-1, 2)}));
	Integer span = Integer(high) - low;
	assert(distance_squared_exact(diagonal.a, diagonal.b) == Rational(2 * span * span));
	Integer huge = Integer(1) << 180;
	BigPoint a{huge, -huge}, b{huge + 3, -huge + 6}, p{huge + 3, -huge};
	Line<Integer> line{a, b};
	auto projected = projection_exact(line, p);
	assert((projected == RPoint{Rational(huge) + fraction(3, 5), Rational(-huge) + fraction(6, 5)}));
	assert(distance_squared_exact(line, p) == fraction(36, 5));
	assert(reflection_exact(line, reflection_exact(line, p)) == to_rational(p));
	Line<Integer> horizontal{{huge - 2, -huge + 1}, {huge + 2, -huge + 1}};
	assert((intersection_exact(line, horizontal) == RPoint{Rational(huge) + fraction(1, 2), Rational(-huge + 1)}));
	assert(distance_squared_exact(BigPoint{huge, huge}, BigPoint{-huge, -huge}) == Rational(8 * huge * huge));
	RPoint fractional{fraction(1, 3), fraction(1, 5)};
	Line<Rational> sloping{fractional, {fraction(7, 3), fraction(11, 5)}};
	auto q = projection_exact(sloping, IPoint{1, 0});
	assert(on_line(sloping, q));
	assert(dot(to_rational(IPoint{1, 0}) - q, sloping.b - sloping.a) == 0);
	assert(intersection_exact(Line<std::int64_t>{{0, 0}, {0, 1}}, Segment<Rational>{{-1, 0}, {1, 0}})
		== std::vector<RPoint>({RPoint{0, 0}}));
	assert(distance_squared_exact(Segment<std::int64_t>{{0, 0}, {1, 0}}, RPoint{fraction(1, 2), fraction(1, 3)}) == fraction(1, 9));
	Integer denominator = Integer(1) << 200;
	Rational tiny = Rational(1) / denominator;
	Line<Rational> almost_parallel{{0, 1}, {1, Rational(2) + tiny}};
	assert((intersection_exact(Line<std::int64_t>{{0, 0}, {1, 1}}, almost_parallel)
		== RPoint{-Rational(denominator), -Rational(denominator)}));
	assert(distance_squared_exact(Line<std::int64_t>{{0, 0}, {1, 0}}, RPoint{0, tiny}) == Rational(tiny * tiny));
}
