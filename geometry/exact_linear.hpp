#ifndef GEOMETRY_EXACT_LINEAR_HPP
#define GEOMETRY_EXACT_LINEAR_HPP

#include "rational.hpp"
#include "linear.hpp"
#include <algorithm>
#include <array>
#include <optional>
#include <vector>

namespace geometry {
namespace exact_linear_detail {
template<ExactCoordinate T>
Line<Rational> convert(Line<T> line) { return {to_rational(line.a), to_rational(line.b)}; }
template<ExactCoordinate T>
Segment<Rational> convert(Segment<T> segment) { return {to_rational(segment.a), to_rational(segment.b)}; }
}

template<ExactCoordinate T, ExactCoordinate U>
RPoint projection_exact(Line<T> line, Point<U> point) {
	auto l = exact_linear_detail::convert(line);
	auto p = to_rational(point), direction = l.b - l.a;
	Rational square = norm2(direction);
	if (!square) return l.a;
	Rational ratio = dot(p - l.a, direction) / square;
	return l.a + direction * ratio;
}

template<ExactCoordinate T, ExactCoordinate U>
RPoint reflection_exact(Line<T> line, Point<U> point) {
	return projection_exact(line, point) * Rational(2) - to_rational(point);
}

template<ExactCoordinate T, ExactCoordinate U>
std::optional<RPoint> intersection_exact(Line<T> line, Line<U> other) {
	auto l = exact_linear_detail::convert(line), m = exact_linear_detail::convert(other);
	if (l.a == l.b) return on_line(m, l.a) ? std::optional<RPoint>(l.a) : std::nullopt;
	if (m.a == m.b) return on_line(l, m.a) ? std::optional<RPoint>(m.a) : std::nullopt;
	auto direction = l.b - l.a, second = m.b - m.a;
	Rational determinant = cross(direction, second);
	if (!determinant) return std::nullopt;
	Rational ratio = cross(m.a - l.a, second) / determinant;
	return l.a + direction * ratio;
}

template<ExactCoordinate T, ExactCoordinate U>
std::vector<RPoint> intersection_exact(Segment<T> segment, Segment<U> other) {
	auto s = exact_linear_detail::convert(segment), t = exact_linear_detail::convert(other);
	if (!intersects(s, t)) return {};
	if (auto p = intersection_exact(Line<Rational>{s.a, s.b}, Line<Rational>{t.a, t.b})) return {*p};
	std::array<RPoint, 4> candidates{s.a, s.b, t.a, t.b};
	std::sort(candidates.begin(), candidates.end());
	std::optional<RPoint> first, last;
	for (const auto &p : candidates) if (on_segment(s, p) && on_segment(t, p)) {
		if (!first) first = p;
		last = p;
	}
	if (!first) return {};
	if (*first == *last) return {*first};
	return {*first, *last};
}

template<ExactCoordinate T, ExactCoordinate U>
std::vector<RPoint> intersection_exact(Line<T> line, Segment<U> segment) {
	auto l = exact_linear_detail::convert(line);
	auto s = exact_linear_detail::convert(segment);
	if (!intersects(l, s)) return {};
	if (auto p = intersection_exact(l, Line<Rational>{s.a, s.b})) return {*p};
	if (s.a == s.b) return {s.a};
	return {s.a, s.b};
}

template<ExactCoordinate T, ExactCoordinate U>
std::vector<RPoint> intersection_exact(Segment<T> segment, Line<U> line) {
	return intersection_exact(line, segment);
}

template<ExactCoordinate T, ExactCoordinate U>
Rational distance_squared_exact(Point<T> point, Point<U> other) {
	return distance_squared(to_rational(point), to_rational(other));
}

template<ExactCoordinate T, ExactCoordinate U>
Rational distance_squared_exact(Line<T> line, Point<U> point) {
	auto l = exact_linear_detail::convert(line);
	auto p = to_rational(point);
	if (l.a == l.b) return distance_squared(l.a, p);
	Rational determinant = cross(l.a, l.b, p);
	return determinant * determinant / distance_squared(l.a, l.b);
}

template<ExactCoordinate T, ExactCoordinate U>
Rational distance_squared_exact(Segment<T> segment, Point<U> point) {
	auto s = exact_linear_detail::convert(segment);
	auto p = to_rational(point), direction = s.b - s.a;
	Rational square = norm2(direction), along = dot(p - s.a, direction);
	if (along <= 0) return distance_squared(s.a, p);
	if (along >= square) return distance_squared(s.b, p);
	Rational determinant = cross(s.a, s.b, p);
	return determinant * determinant / square;
}

template<ExactCoordinate T, ExactCoordinate U>
Rational distance_squared_exact(Point<T> point, Line<U> line) { return distance_squared_exact(line, point); }

template<ExactCoordinate T, ExactCoordinate U>
Rational distance_squared_exact(Point<T> point, Segment<U> segment) { return distance_squared_exact(segment, point); }

template<ExactCoordinate T, ExactCoordinate U>
Rational distance_squared_exact(Line<T> line, Line<U> other) {
	auto l = exact_linear_detail::convert(line), m = exact_linear_detail::convert(other);
	if (intersects(l, m)) return 0;
	return l.a == l.b ? distance_squared_exact(m, l.a) : distance_squared_exact(l, m.a);
}

template<ExactCoordinate T, ExactCoordinate U>
Rational distance_squared_exact(Line<T> line, Segment<U> segment) {
	auto l = exact_linear_detail::convert(line);
	auto s = exact_linear_detail::convert(segment);
	if (intersects(l, s)) return 0;
	return l.a == l.b ? distance_squared_exact(s, l.a)
		: std::min(distance_squared_exact(l, s.a), distance_squared_exact(l, s.b));
}

template<ExactCoordinate T, ExactCoordinate U>
Rational distance_squared_exact(Segment<T> segment, Line<U> line) { return distance_squared_exact(line, segment); }

template<ExactCoordinate T, ExactCoordinate U>
Rational distance_squared_exact(Segment<T> segment, Segment<U> other) {
	auto s = exact_linear_detail::convert(segment), t = exact_linear_detail::convert(other);
	if (intersects(s, t)) return 0;
	return std::min({distance_squared_exact(s, t.a), distance_squared_exact(s, t.b),
		distance_squared_exact(t, s.a), distance_squared_exact(t, s.b)});
}

template<ExactCoordinate T, ExactCoordinate U>
std::optional<Line<Rational>> perpendicular_bisector_exact(Point<T> point, Point<U> other) {
	auto a = to_rational(point), b = to_rational(other);
	if (a == b) return std::nullopt;
	auto middle = (a + b) / Rational(2), direction = rotate90(b - a);
	return Line<Rational>{middle, middle + direction};
}
}

#endif
