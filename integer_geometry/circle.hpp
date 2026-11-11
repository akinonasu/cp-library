#ifndef INTEGER_GEOMETRY_CIRCLE_HPP
#define INTEGER_GEOMETRY_CIRCLE_HPP

#include "linear.hpp"
#include "../geometry/exact_circle.hpp"
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace integer_geometry {
struct Circle { RPoint center{}; Rational radius_squared = 0; };
enum class CircleRelation { separate, external_tangent, intersecting, internal_tangent, contained, coincident };

namespace circle_detail {
inline geometry::ExactCircle convert(const Circle &circle) {
	return {integer_geometry_detail::to_geometry(circle.center), circle.radius_squared};
}
inline Circle convert(const geometry::ExactCircle &circle) {
	return {integer_geometry_detail::from_geometry(circle.center), circle.radius_squared};
}
}

template<Coordinate T>
Circle make_circle(const BasicPoint<T> &center, Rational radius) {
	return circle_detail::convert(geometry::make_exact_circle(integer_geometry_detail::to_geometry(center), std::move(radius)));
}
template<Coordinate T>
Location contains(const Circle &circle, const BasicPoint<T> &point) {
	return integer_geometry_detail::from_geometry(geometry::contains(circle_detail::convert(circle), integer_geometry_detail::to_geometry(point)));
}
inline CircleRelation relation(const Circle &a, const Circle &b) {
	switch (geometry::relation(circle_detail::convert(a), circle_detail::convert(b))) {
		case geometry::CircleRelation::separate: return CircleRelation::separate;
		case geometry::CircleRelation::external_tangent: return CircleRelation::external_tangent;
		case geometry::CircleRelation::intersecting: return CircleRelation::intersecting;
		case geometry::CircleRelation::internal_tangent: return CircleRelation::internal_tangent;
		case geometry::CircleRelation::contained: return CircleRelation::contained;
		case geometry::CircleRelation::coincident: return CircleRelation::coincident;
	}
	return CircleRelation::coincident;
}
inline bool intersects(const Circle &a, const Circle &b) {
	return geometry::intersects(circle_detail::convert(a), circle_detail::convert(b));
}
template<Coordinate T>
bool intersects(const Circle &circle, const Line<T> &line) {
	return geometry::intersects(circle_detail::convert(circle), linear_detail::convert<T>(line));
}
template<Coordinate T>
bool intersects(const Line<T> &line, const Circle &circle) { return intersects(circle, line); }
template<Coordinate T>
bool intersects(const Circle &circle, const Segment<T> &segment) {
	return geometry::intersects(circle_detail::convert(circle), linear_detail::convert<T>(segment));
}
template<Coordinate T>
bool intersects(const Segment<T> &segment, const Circle &circle) { return intersects(circle, segment); }
template<Coordinate T, Coordinate U, Coordinate V>
std::optional<Circle> circumcircle(const BasicPoint<T> &a, const BasicPoint<U> &b, const BasicPoint<V> &c) {
	auto circle = geometry::circumcircle_exact(linear_detail::convert<Rational>(a), linear_detail::convert<Rational>(b), linear_detail::convert<Rational>(c));
	if (!circle) return std::nullopt;
	return circle_detail::convert(*circle);
}
template<Coordinate T>
std::optional<Circle> minimum_enclosing_circle(const std::vector<BasicPoint<T>> &points, std::uint64_t seed = 712367821) {
	auto circle = geometry::minimum_enclosing_circle_exact(integer_geometry_detail::to_geometry(points), seed);
	if (!circle) return std::nullopt;
	return circle_detail::convert(*circle);
}
}

#endif
