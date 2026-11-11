#ifndef INTEGER_GEOMETRY_LINEAR_HPP
#define INTEGER_GEOMETRY_LINEAR_HPP

#include "../components/integer_geometry_bridge.hpp"
#include "../geometry/exact_linear.hpp"
#include <optional>
#include <type_traits>
#include <vector>

namespace integer_geometry {
template<Coordinate T = Integer> struct Line { BasicPoint<T> a, b; };
template<Coordinate T = Integer> struct Segment { BasicPoint<T> a, b; };
template<Coordinate T> Line(BasicPoint<T>, BasicPoint<T>) -> Line<T>;
template<Coordinate T> Segment(BasicPoint<T>, BasicPoint<T>) -> Segment<T>;

namespace linear_detail {
template<class T, class U>
using Common = std::conditional_t<std::is_same_v<T, Integer> && std::is_same_v<U, Integer>, Integer, Rational>;
template<Coordinate C, Coordinate T>
geometry::Point<C> convert(const BasicPoint<T> &p) { return {C(p.x), C(p.y)}; }
template<Coordinate C, Coordinate T>
geometry::Line<C> convert(const Line<T> &line) { return {convert<C>(line.a), convert<C>(line.b)}; }
template<Coordinate C, Coordinate T>
geometry::Segment<C> convert(const Segment<T> &segment) { return {convert<C>(segment.a), convert<C>(segment.b)}; }
}

template<Coordinate T, Coordinate U>
bool parallel(const Line<T> &a, const Line<U> &b) {
	using C = linear_detail::Common<T, U>;
	return geometry::parallel(linear_detail::convert<C>(a), linear_detail::convert<C>(b));
}
template<Coordinate T, Coordinate U>
bool orthogonal(const Line<T> &a, const Line<U> &b) {
	using C = linear_detail::Common<T, U>;
	return geometry::orthogonal(linear_detail::convert<C>(a), linear_detail::convert<C>(b));
}
template<Coordinate T, Coordinate U>
bool on_line(const Line<T> &line, const BasicPoint<U> &point) {
	using C = linear_detail::Common<T, U>;
	return geometry::on_line(linear_detail::convert<C>(line), linear_detail::convert<C>(point));
}
template<Coordinate T, Coordinate U>
bool on_segment(const Segment<T> &segment, const BasicPoint<U> &point) {
	using C = linear_detail::Common<T, U>;
	return geometry::on_segment(linear_detail::convert<C>(segment), linear_detail::convert<C>(point));
}
template<Coordinate T, Coordinate U>
bool intersects(const Line<T> &a, const Line<U> &b) {
	using C = linear_detail::Common<T, U>;
	return geometry::intersects(linear_detail::convert<C>(a), linear_detail::convert<C>(b));
}
template<Coordinate T, Coordinate U>
bool intersects(const Line<T> &line, const Segment<U> &segment) {
	using C = linear_detail::Common<T, U>;
	return geometry::intersects(linear_detail::convert<C>(line), linear_detail::convert<C>(segment));
}
template<Coordinate T, Coordinate U>
bool intersects(const Segment<T> &segment, const Line<U> &line) { return intersects(line, segment); }
template<Coordinate T, Coordinate U>
bool intersects(const Segment<T> &a, const Segment<U> &b) {
	using C = linear_detail::Common<T, U>;
	return geometry::intersects(linear_detail::convert<C>(a), linear_detail::convert<C>(b));
}

template<Coordinate T, Coordinate U>
RPoint projection(const Line<T> &line, const BasicPoint<U> &point) {
	return integer_geometry_detail::from_geometry(geometry::projection_exact(linear_detail::convert<T>(line), integer_geometry_detail::to_geometry(point)));
}
template<Coordinate T, Coordinate U>
RPoint reflection(const Line<T> &line, const BasicPoint<U> &point) {
	return integer_geometry_detail::from_geometry(geometry::reflection_exact(linear_detail::convert<T>(line), integer_geometry_detail::to_geometry(point)));
}
template<Coordinate T, Coordinate U>
std::optional<RPoint> intersection(const Line<T> &a, const Line<U> &b) {
	auto point = geometry::intersection_exact(linear_detail::convert<T>(a), linear_detail::convert<U>(b));
	if (!point) return std::nullopt;
	return integer_geometry_detail::from_geometry(*point);
}
template<Coordinate T, Coordinate U>
std::vector<RPoint> intersection(const Segment<T> &a, const Segment<U> &b) {
	return integer_geometry_detail::from_geometry(geometry::intersection_exact(linear_detail::convert<T>(a), linear_detail::convert<U>(b)));
}
template<Coordinate T, Coordinate U>
std::vector<RPoint> intersection(const Line<T> &line, const Segment<U> &segment) {
	return integer_geometry_detail::from_geometry(geometry::intersection_exact(linear_detail::convert<T>(line), linear_detail::convert<U>(segment)));
}
template<Coordinate T, Coordinate U>
std::vector<RPoint> intersection(const Segment<T> &segment, const Line<U> &line) { return intersection(line, segment); }

template<Coordinate T, Coordinate U>
Rational distance_squared(const Line<T> &line, const BasicPoint<U> &point) {
	return geometry::distance_squared_exact(linear_detail::convert<T>(line), integer_geometry_detail::to_geometry(point));
}
template<Coordinate T, Coordinate U>
Rational distance_squared(const Segment<T> &segment, const BasicPoint<U> &point) {
	return geometry::distance_squared_exact(linear_detail::convert<T>(segment), integer_geometry_detail::to_geometry(point));
}
template<Coordinate T, Coordinate U>
Rational distance_squared(const BasicPoint<T> &point, const Line<U> &line) { return distance_squared(line, point); }
template<Coordinate T, Coordinate U>
Rational distance_squared(const BasicPoint<T> &point, const Segment<U> &segment) { return distance_squared(segment, point); }
template<Coordinate T, Coordinate U>
Rational distance_squared(const Line<T> &a, const Line<U> &b) {
	return geometry::distance_squared_exact(linear_detail::convert<T>(a), linear_detail::convert<U>(b));
}
template<Coordinate T, Coordinate U>
Rational distance_squared(const Line<T> &line, const Segment<U> &segment) {
	return geometry::distance_squared_exact(linear_detail::convert<T>(line), linear_detail::convert<U>(segment));
}
template<Coordinate T, Coordinate U>
Rational distance_squared(const Segment<T> &segment, const Line<U> &line) { return distance_squared(line, segment); }
template<Coordinate T, Coordinate U>
Rational distance_squared(const Segment<T> &a, const Segment<U> &b) {
	return geometry::distance_squared_exact(linear_detail::convert<T>(a), linear_detail::convert<U>(b));
}
template<Coordinate T, Coordinate U>
std::optional<Line<Rational>> perpendicular_bisector(const BasicPoint<T> &a, const BasicPoint<U> &b) {
	auto line = geometry::perpendicular_bisector_exact(integer_geometry_detail::to_geometry(a), integer_geometry_detail::to_geometry(b));
	if (!line) return std::nullopt;
	return Line<Rational>{integer_geometry_detail::from_geometry(line->a), integer_geometry_detail::from_geometry(line->b)};
}
}

#endif
