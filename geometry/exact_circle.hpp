#ifndef GEOMETRY_EXACT_CIRCLE_HPP
#define GEOMETRY_EXACT_CIRCLE_HPP

#include "circle.hpp"
#include "exact_linear.hpp"
#include "../components/exact_geometry_integer.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

namespace geometry {
struct ExactCircle { RPoint center{}; Rational radius_squared = 0; };

template<ExactCoordinate T>
ExactCircle make_exact_circle(Point<T> center, Rational radius) {
	assert(radius >= 0);
	return {to_rational(center), Rational(radius * radius)};
}

template<ExactCoordinate T>
Location contains(const ExactCircle &circle, Point<T> point) {
	assert(circle.radius_squared >= 0);
	if constexpr (exact_integer_detail::coordinate<T>) {
		Integer x_denominator = denominator(circle.center.x), y_denominator = denominator(circle.center.y);
		Integer x = (Integer(point.x) * x_denominator - numerator(circle.center.x)) * y_denominator;
		Integer y = (Integer(point.y) * y_denominator - numerator(circle.center.y)) * x_denominator;
		Integer scale = x_denominator * y_denominator;
		Integer difference = (x * x + y * y) * denominator(circle.radius_squared)
			- numerator(circle.radius_squared) * scale * scale;
		return difference > 0 ? Location::outside : difference < 0 ? Location::inside : Location::boundary;
	} else {
		Rational square = distance_squared(circle.center, to_rational(point));
		if (square > circle.radius_squared) return Location::outside;
		return square < circle.radius_squared ? Location::inside : Location::boundary;
	}
}

inline CircleRelation relation(const ExactCircle &a, const ExactCircle &b) {
	assert(a.radius_squared >= 0 && b.radius_squared >= 0);
	Rational square = distance_squared(a.center, b.center);
	if (square == 0 && a.radius_squared == b.radius_squared) return CircleRelation::coincident;
	Rational difference = square - a.radius_squared - b.radius_squared;
	Rational discriminant = difference * difference - 4 * a.radius_squared * b.radius_squared;
	if (discriminant < 0) return CircleRelation::intersecting;
	if (difference >= 0)
		return discriminant == 0 ? CircleRelation::external_tangent : CircleRelation::separate;
	return discriminant == 0 ? CircleRelation::internal_tangent : CircleRelation::contained;
}

inline bool intersects(const ExactCircle &a, const ExactCircle &b) {
	auto type = relation(a, b);
	return type != CircleRelation::separate && type != CircleRelation::contained;
}

template<ExactCoordinate T>
bool intersects(const ExactCircle &circle, Line<T> line) {
	assert(circle.radius_squared >= 0);
	Line<Rational> converted{to_rational(line.a), to_rational(line.b)};
	if (converted.a == converted.b) return contains(circle, converted.a) == Location::boundary;
	return distance_squared_exact(converted, circle.center) <= circle.radius_squared;
}
template<ExactCoordinate T>
bool intersects(Line<T> line, const ExactCircle &circle) { return intersects(circle, line); }

template<ExactCoordinate T>
bool intersects(const ExactCircle &circle, Segment<T> segment) {
	assert(circle.radius_squared >= 0);
	Segment<Rational> converted{to_rational(segment.a), to_rational(segment.b)};
	Rational furthest = std::max(distance_squared(circle.center, converted.a), distance_squared(circle.center, converted.b));
	return furthest >= circle.radius_squared && distance_squared_exact(converted, circle.center) <= circle.radius_squared;
}
template<ExactCoordinate T>
bool intersects(Segment<T> segment, const ExactCircle &circle) { return intersects(circle, segment); }

template<ExactCoordinate T>
std::optional<ExactCircle> circumcircle_exact(Point<T> a, Point<T> b, Point<T> c) {
	if constexpr (exact_integer_detail::coordinate<T>) {
		auto circle = exact_integer_detail::circumcircle(exact_integer_detail::convert(a),
			exact_integer_detail::convert(b), exact_integer_detail::convert(c));
		if (!circle) return std::nullopt;
		return ExactCircle{exact_integer_detail::quotient(circle->center, circle->denominator),
			Rational(circle->square, Integer(circle->denominator * circle->denominator))};
	}
	RPoint origin = to_rational(a), u = to_rational(b) - origin, v = to_rational(c) - origin;
	Rational determinant = 2 * cross(u, v);
	if (determinant == 0) return std::nullopt;
	Rational u2 = norm2(u), v2 = norm2(v);
	RPoint offset{Rational((u2 * v.y - v2 * u.y) / determinant), Rational((u.x * v2 - v.x * u2) / determinant)};
	return ExactCircle{origin + offset, norm2(offset)};
}

template<ExactCoordinate T>
std::optional<ExactCircle> minimum_enclosing_circle_exact(const std::vector<Point<T>> &points, std::uint64_t seed = 712367821) {
	if (points.empty()) return std::nullopt;
	if constexpr (exact_integer_detail::coordinate<T>) {
		auto p = points;
		std::mt19937_64 random(seed);
		std::shuffle(p.begin(), p.end(), random);
		exact_integer_detail::Circle result{exact_integer_detail::convert(p[0]), 1, 0};
		for (std::size_t i = 1; i < p.size(); ++i) if (result.indicator(p[i]) > 0) {
			auto a = exact_integer_detail::convert(p[i]);
			result = {a, 1, 0};
			for (std::size_t j = 0; j < i; ++j) if (result.indicator(p[j]) > 0) {
				auto b = exact_integer_detail::convert(p[j]);
				result = exact_integer_detail::diameter(a, b);
				for (std::size_t k = 0; k < j; ++k) if (result.indicator(p[k]) > 0) {
					auto c = exact_integer_detail::convert(p[k]);
					if (auto circle = exact_integer_detail::circumcircle(a, b, c)) result = std::move(*circle);
					else {
						std::array<exact_integer_detail::Circle, 3> candidates{
							exact_integer_detail::diameter(a, b), exact_integer_detail::diameter(a, c), exact_integer_detail::diameter(b, c)};
						result = *std::max_element(candidates.begin(), candidates.end(), [](const auto &first, const auto &second) {
							return first.square < second.square;
						});
					}
				}
			}
		}
		return ExactCircle{exact_integer_detail::quotient(result.center, result.denominator),
			Rational(result.square, Integer(result.denominator * result.denominator))};
	}
	std::vector<RPoint> p;
	p.reserve(points.size());
	for (const auto &point : points) p.push_back(to_rational(point));
	std::mt19937_64 random(seed);
	std::shuffle(p.begin(), p.end(), random);
	auto diameter = [](const RPoint &a, const RPoint &b) -> ExactCircle {
		return {(a + b) / Rational(2), Rational(distance_squared(a, b) / 4)};
	};
	ExactCircle result{p[0], 0};
	for (std::size_t i = 1; i < p.size(); ++i) if (contains(result, p[i]) == Location::outside) {
		result = {p[i], 0};
		for (std::size_t j = 0; j < i; ++j) if (contains(result, p[j]) == Location::outside) {
			result = diameter(p[i], p[j]);
			for (std::size_t k = 0; k < j; ++k) if (contains(result, p[k]) == Location::outside) {
				if (auto circle = circumcircle_exact(p[i], p[j], p[k])) result = *circle;
				else {
					std::array<ExactCircle, 3> candidates{diameter(p[i], p[j]), diameter(p[i], p[k]), diameter(p[j], p[k])};
					result = *std::max_element(candidates.begin(), candidates.end(), [](const ExactCircle &a, const ExactCircle &b) {
						return a.radius_squared < b.radius_squared;
					});
				}
			}
		}
	}
	return result;
}
}

#endif
