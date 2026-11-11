#ifndef INTEGER_GEOMETRY_BRIDGE_HPP
#define INTEGER_GEOMETRY_BRIDGE_HPP

#include "../integer_geometry/point.hpp"
#include "../geometry/point.hpp"
#include <vector>

namespace integer_geometry_detail {
template<integer_geometry::Coordinate T>
geometry::Point<T> to_geometry(const integer_geometry::BasicPoint<T> &point) { return {point.x, point.y}; }

template<integer_geometry::Coordinate T>
integer_geometry::BasicPoint<T> from_geometry(const geometry::Point<T> &point) { return {point.x, point.y}; }

template<integer_geometry::Coordinate T>
std::vector<geometry::Point<T>> to_geometry(const std::vector<integer_geometry::BasicPoint<T>> &points) {
	std::vector<geometry::Point<T>> result;
	result.reserve(points.size());
	for (const auto &point : points) result.push_back(to_geometry(point));
	return result;
}

template<integer_geometry::Coordinate T>
std::vector<integer_geometry::BasicPoint<T>> from_geometry(const std::vector<geometry::Point<T>> &points) {
	std::vector<integer_geometry::BasicPoint<T>> result;
	result.reserve(points.size());
	for (const auto &point : points) result.push_back(from_geometry(point));
	return result;
}

inline integer_geometry::Location from_geometry(geometry::Location location) {
	return static_cast<integer_geometry::Location>(location);
}
}

#endif
