#ifndef INTEGER_GEOMETRY_CIRCLE_HPP
#define INTEGER_GEOMETRY_CIRCLE_HPP

#include "linear.hpp"
#include <algorithm>
#include <cassert>
#include <concepts>
#include <type_traits>

namespace integer_geometry {
template<Coordinate T = Integer> struct BasicCircle { BasicPoint<T> center{}; T radius = 0; };
template<Coordinate T> BasicCircle(BasicPoint<T>, std::type_identity_t<T>) -> BasicCircle<T>;
using Circle = BasicCircle<Integer>;
enum class CircleRelation { separate, external_tangent, intersecting, internal_tangent, contained, coincident };

template<Coordinate T>
constexpr BasicCircle<T> make_circle(BasicPoint<T> center, std::type_identity_t<T> radius) {
	assert(radius >= 0);
	return {center, radius};
}
template<Coordinate T, std::floating_point U>
BasicCircle<T> make_circle(BasicPoint<T>, U) = delete;

template<Coordinate T, Coordinate U>
constexpr Location contains(const BasicCircle<T> &circle, const BasicPoint<U> &point) {
	assert(circle.radius >= 0);
	Wide square = linear_detail::squared_distance(circle.center, point), radius2 = Wide(circle.radius) * circle.radius;
	return square > radius2 ? Location::outside : square < radius2 ? Location::inside : Location::boundary;
}
template<Coordinate T, Coordinate U>
constexpr CircleRelation relation(const BasicCircle<T> &a, const BasicCircle<U> &b) {
	assert(a.radius >= 0 && b.radius >= 0);
	Wide square = linear_detail::squared_distance(a.center, b.center);
	if (!square && a.radius == b.radius) return CircleRelation::coincident;
	Wide outer = Wide(a.radius) + b.radius, inner = Wide(a.radius) - b.radius;
	outer *= outer; inner *= inner;
	if (square > outer) return CircleRelation::separate;
	if (square == outer) return CircleRelation::external_tangent;
	if (square > inner) return CircleRelation::intersecting;
	return square == inner ? CircleRelation::internal_tangent : CircleRelation::contained;
}
template<Coordinate T, Coordinate U>
constexpr bool intersects(const BasicCircle<T> &a, const BasicCircle<U> &b) {
	auto type = relation(a, b);
	return type != CircleRelation::separate && type != CircleRelation::contained;
}
template<Coordinate T, Coordinate U>
constexpr bool intersects(const BasicCircle<T> &circle, const Line<U> &line) {
	assert(circle.radius >= 0);
	Wide x = Wide(line.b.x) - line.a.x, y = Wide(line.b.y) - line.a.y;
	Wide square = x * x + y * y;
	if (!square) return contains(circle, line.a) == Location::boundary;
	Wide determinant = linear_detail::side(line.a, line.b, circle.center);
	return determinant * determinant <= Wide(circle.radius) * circle.radius * square;
}
template<Coordinate T, Coordinate U>
constexpr bool intersects(const Line<T> &line, const BasicCircle<U> &circle) { return intersects(circle, line); }
template<Coordinate T, Coordinate U>
constexpr bool intersects(const BasicCircle<T> &circle, const Segment<U> &segment) {
	assert(circle.radius >= 0);
	Wide a = linear_detail::squared_distance(circle.center, segment.a), b = linear_detail::squared_distance(circle.center, segment.b);
	Wide radius2 = Wide(circle.radius) * circle.radius;
	if (std::max(a, b) < radius2) return false;
	Wide x = Wide(segment.b.x) - segment.a.x, y = Wide(segment.b.y) - segment.a.y;
	Wide square = x * x + y * y;
	Wide along = (Wide(circle.center.x) - segment.a.x) * x + (Wide(circle.center.y) - segment.a.y) * y;
	if (along <= 0) return a <= radius2;
	if (along >= square) return b <= radius2;
	Wide determinant = linear_detail::side(segment.a, segment.b, circle.center);
	return determinant * determinant <= radius2 * square;
}
template<Coordinate T, Coordinate U>
constexpr bool intersects(const Segment<T> &segment, const BasicCircle<U> &circle) { return intersects(circle, segment); }
}

#endif
