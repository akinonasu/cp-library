#ifndef INTEGER_GEOMETRY_LINEAR_HPP
#define INTEGER_GEOMETRY_LINEAR_HPP

#include "point.hpp"
#include <algorithm>

namespace integer_geometry {
template<Coordinate T = Integer> struct Line { BasicPoint<T> a, b; };
template<Coordinate T = Integer> struct Segment { BasicPoint<T> a, b; };
template<Coordinate T> Line(BasicPoint<T>, BasicPoint<T>) -> Line<T>;
template<Coordinate T> Segment(BasicPoint<T>, BasicPoint<T>) -> Segment<T>;

namespace linear_detail {
template<Coordinate T, Coordinate U>
constexpr Wide side(const BasicPoint<T> &a, const BasicPoint<T> &b, const BasicPoint<U> &p) {
	return (Wide(b.x) - a.x) * (Wide(p.y) - a.y) - (Wide(b.y) - a.y) * (Wide(p.x) - a.x);
}
template<Coordinate T, Coordinate U>
constexpr Wide squared_distance(const BasicPoint<T> &a, const BasicPoint<U> &b) {
	Wide x = Wide(a.x) - b.x, y = Wide(a.y) - b.y;
	return x * x + y * y;
}
}

template<Coordinate T, Coordinate U>
constexpr bool parallel(const Line<T> &a, const Line<U> &b) {
	return (Wide(a.b.x) - a.a.x) * (Wide(b.b.y) - b.a.y)
		== (Wide(a.b.y) - a.a.y) * (Wide(b.b.x) - b.a.x);
}
template<Coordinate T, Coordinate U>
constexpr bool orthogonal(const Line<T> &a, const Line<U> &b) {
	return (Wide(a.b.x) - a.a.x) * (Wide(b.b.x) - b.a.x)
		+ (Wide(a.b.y) - a.a.y) * (Wide(b.b.y) - b.a.y) == 0;
}
template<Coordinate T, Coordinate U>
constexpr bool on_line(const Line<T> &line, const BasicPoint<U> &point) {
	if (line.a == line.b) return line.a.x == point.x && line.a.y == point.y;
	return linear_detail::side(line.a, line.b, point) == 0;
}
template<Coordinate T, Coordinate U>
constexpr bool on_segment(const Segment<T> &segment, const BasicPoint<U> &point) {
	return linear_detail::side(segment.a, segment.b, point) == 0
		&& std::min(segment.a.x, segment.b.x) <= point.x && point.x <= std::max(segment.a.x, segment.b.x)
		&& std::min(segment.a.y, segment.b.y) <= point.y && point.y <= std::max(segment.a.y, segment.b.y);
}
template<Coordinate T, Coordinate U>
constexpr bool intersects(const Line<T> &a, const Line<U> &b) {
	if (a.a == a.b) return on_line(b, a.a);
	if (b.a == b.b) return on_line(a, b.a);
	return !parallel(a, b) || on_line(a, b.a);
}
template<Coordinate T, Coordinate U>
constexpr bool intersects(const Line<T> &line, const Segment<U> &segment) {
	if (line.a == line.b) return on_segment(segment, line.a);
	return sign(linear_detail::side(line.a, line.b, segment.a))
		* sign(linear_detail::side(line.a, line.b, segment.b)) <= 0;
}
template<Coordinate T, Coordinate U>
constexpr bool intersects(const Segment<T> &segment, const Line<U> &line) { return intersects(line, segment); }
template<Coordinate T, Coordinate U>
constexpr bool intersects(const Segment<T> &a, const Segment<U> &b) {
	int first = sign(linear_detail::side(a.a, a.b, b.a)), second = sign(linear_detail::side(a.a, a.b, b.b));
	int third = sign(linear_detail::side(b.a, b.b, a.a)), fourth = sign(linear_detail::side(b.a, b.b, a.b));
	return (!first && on_segment(a, b.a)) || (!second && on_segment(a, b.b))
		|| (!third && on_segment(b, a.a)) || (!fourth && on_segment(b, a.b))
		|| (first * second < 0 && third * fourth < 0);
}
}

#endif
