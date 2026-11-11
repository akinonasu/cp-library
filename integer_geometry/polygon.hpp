#ifndef INTEGER_GEOMETRY_POLYGON_HPP
#define INTEGER_GEOMETRY_POLYGON_HPP

#include "linear.hpp"
#include <cstdint>
#include <vector>

namespace integer_geometry {
template<Coordinate T>
Wide signed_area2(const std::vector<BasicPoint<T>> &polygon) {
	Wide result = 0;
	for (std::size_t i = 2; i < polygon.size(); ++i) result += cross(polygon[0], polygon[i - 1], polygon[i]);
	return result;
}
template<Coordinate T>
Wide doubled_area(const std::vector<BasicPoint<T>> &polygon) {
	Wide result = signed_area2(polygon);
	return result < 0 ? -result : result;
}
template<Coordinate T>
bool is_convex(const std::vector<BasicPoint<T>> &polygon, bool strict = false) {
	if (polygon.size() < 3) return !strict;
	int direction = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		int turn = orientation(polygon[i], polygon[(i + 1) % polygon.size()], polygon[(i + 2) % polygon.size()]);
		if (!turn) { if (strict) return false; }
		else if (direction && turn != direction) return false;
		else direction = turn;
	}
	return true;
}
template<Coordinate T, Coordinate U>
Location contains(const std::vector<BasicPoint<T>> &polygon, const BasicPoint<U> &point) {
	std::int64_t winding = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		const auto &a = polygon[i], &b = polygon[(i + 1) % polygon.size()];
		if (on_segment(Segment<T>{a, b}, point)) return Location::boundary;
		Wide turn = linear_detail::side(a, b, point);
		if (a.y <= point.y && point.y < b.y && turn > 0) ++winding;
		if (b.y <= point.y && point.y < a.y && turn < 0) --winding;
	}
	return winding ? Location::inside : Location::outside;
}
}

#endif
