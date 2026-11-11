#ifndef INTEGER_GEOMETRY_POLYGON_HPP
#define INTEGER_GEOMETRY_POLYGON_HPP

#include "linear.hpp"
#include "../geometry/exact_polygon.hpp"
#include <optional>
#include <vector>

namespace integer_geometry {
template<Coordinate T>
T signed_area2(const std::vector<BasicPoint<T>> &polygon) {
	T result = 0;
	for (std::size_t i = 2; i < polygon.size(); ++i) result += cross(polygon[0], polygon[i - 1], polygon[i]);
	return result;
}
template<Coordinate T>
Rational area(const std::vector<BasicPoint<T>> &polygon) {
	Rational value = signed_area2(polygon);
	return (value < 0 ? -value : value) / 2;
}
template<Coordinate T>
bool is_convex(const std::vector<BasicPoint<T>> &polygon, bool strict = false) {
	return geometry::is_convex(integer_geometry_detail::to_geometry(polygon), strict);
}
template<Coordinate T, Coordinate U>
Location contains(const std::vector<BasicPoint<T>> &polygon, const BasicPoint<U> &point) {
	if constexpr (std::is_same_v<T, U>)
		return integer_geometry_detail::from_geometry(geometry::contains(integer_geometry_detail::to_geometry(polygon), integer_geometry_detail::to_geometry(point)));
	else {
		std::vector<geometry::RPoint> converted;
		converted.reserve(polygon.size());
		for (const auto &p : polygon) converted.push_back(linear_detail::convert<Rational>(p));
		return integer_geometry_detail::from_geometry(geometry::contains(converted, linear_detail::convert<Rational>(point)));
	}
}
template<Coordinate T>
std::optional<RPoint> centroid(const std::vector<BasicPoint<T>> &polygon) {
	auto point = geometry::centroid_exact(integer_geometry_detail::to_geometry(polygon));
	if (!point) return std::nullopt;
	return integer_geometry_detail::from_geometry(*point);
}
template<Coordinate T, Coordinate U>
std::vector<RPoint> convex_cut(const std::vector<BasicPoint<T>> &polygon, const Line<U> &line) {
	return integer_geometry_detail::from_geometry(geometry::convex_cut_exact(integer_geometry_detail::to_geometry(polygon), linear_detail::convert<U>(line)));
}
}

#endif
