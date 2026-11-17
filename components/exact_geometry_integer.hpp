#ifndef EXACT_GEOMETRY_INTEGER_HPP
#define EXACT_GEOMETRY_INTEGER_HPP

#include "../geometry/rational.hpp"
#include <optional>
#include <type_traits>
#include <utility>

namespace geometry::exact_integer_detail {
template<class T>
inline constexpr bool coordinate = integer<T> || std::is_same_v<T, Integer>;

template<class T>
BigPoint convert(const Point<T> &point) { return {Integer(point.x), Integer(point.y)}; }

inline RPoint quotient(const BigPoint &point, const Integer &denominator) {
	return {Rational(point.x, denominator), Rational(point.y, denominator)};
}

inline std::pair<BigPoint, Integer> projection(const BigPoint &a, const BigPoint &b, const BigPoint &point) {
	BigPoint direction = b - a;
	Integer square = norm2(direction);
	if (square == 0) return {a, 1};
	Integer along = dot(point - a, direction);
	return {a * square + direction * along, square};
}

inline Rational segment_distance(const BigPoint &a, const BigPoint &b, const BigPoint &point) {
	BigPoint direction = b - a;
	Integer square = norm2(direction), along = dot(point - a, direction);
	if (along <= 0) return Rational(distance_squared(a, point));
	if (along >= square) return Rational(distance_squared(b, point));
	Integer determinant = cross(a, b, point);
	return Rational(Integer(determinant * determinant), square);
}

struct Circle {
	BigPoint center;
	Integer denominator = 1, square = 0;
	template<class T>
	Integer indicator(const Point<T> &point) const {
		Integer x = Integer(point.x) * denominator - center.x;
		Integer y = Integer(point.y) * denominator - center.y;
		return x * x + y * y - square;
	}
};

inline Circle diameter(const BigPoint &a, const BigPoint &b) {
	return {a + b, 2, distance_squared(a, b)};
}

inline std::optional<Circle> circumcircle(const BigPoint &a, const BigPoint &b, const BigPoint &c) {
	BigPoint u = b - a, v = c - a;
	Integer denominator = 2 * cross(u, v);
	if (denominator == 0) return std::nullopt;
	Integer u2 = norm2(u), v2 = norm2(v);
	BigPoint offset{Integer(u2 * v.y - v2 * u.y), Integer(u.x * v2 - v.x * u2)};
	return Circle{a * denominator + offset, denominator, norm2(offset)};
}
}

#endif
