#ifndef INTEGER_GEOMETRY_CONVEX_HPP
#define INTEGER_GEOMETRY_CONVEX_HPP

#include "../components/integer_geometry_bridge.hpp"
#include "../geometry/convex.hpp"
#include <optional>
#include <vector>

namespace integer_geometry {
template<Coordinate T>
struct PointPair {
	int first, second;
	Wide squared_distance;
};

template<Coordinate T>
std::vector<int> convex_hull_indices(const std::vector<BasicPoint<T>> &points, bool keep_collinear = false) {
	return geometry::convex_hull_indices(integer_geometry_detail::to_geometry(points), keep_collinear);
}

template<Coordinate T>
std::vector<BasicPoint<T>> convex_hull(const std::vector<BasicPoint<T>> &points, bool keep_collinear = false) {
	return integer_geometry_detail::from_geometry(geometry::convex_hull(integer_geometry_detail::to_geometry(points), keep_collinear));
}

template<Coordinate T>
Location convex_contains(const std::vector<BasicPoint<T>> &polygon, const BasicPoint<T> &point) {
	if (polygon.empty()) return Location::outside;
	auto on_segment = [&](const BasicPoint<T> &a, const BasicPoint<T> &b) {
		return orientation(a, b, point) == 0
			&& std::min(a.x, b.x) <= point.x && point.x <= std::max(a.x, b.x)
			&& std::min(a.y, b.y) <= point.y && point.y <= std::max(a.y, b.y);
	};
	if (polygon.size() <= 2) return on_segment(polygon.front(), polygon.back()) ? Location::boundary : Location::outside;
	int first = orientation(polygon[0], polygon[1], point), last = orientation(polygon[0], polygon.back(), point);
	if (first < 0 || last > 0) return Location::outside;
	if (!first) return on_segment(polygon[0], polygon[1]) ? Location::boundary : Location::outside;
	if (!last) return on_segment(polygon[0], polygon.back()) ? Location::boundary : Location::outside;
	std::size_t l = 1, r = polygon.size() - 1;
	while (r - l > 1) {
		std::size_t m = (l + r) / 2;
		if (orientation(polygon[0], polygon[m], point) >= 0) l = m;
		else r = m;
	}
	int turn = orientation(polygon[l], polygon[r], point);
	return turn > 0 ? Location::inside : !turn && on_segment(polygon[l], polygon[r]) ? Location::boundary : Location::outside;
}

template<Coordinate T>
std::optional<PointPair<T>> convex_diameter(const std::vector<BasicPoint<T>> &polygon) {
	auto result = geometry::convex_diameter(integer_geometry_detail::to_geometry(polygon));
	if (!result) return std::nullopt;
	return PointPair<T>{result->first, result->second, std::move(result->squared_distance)};
}

template<Coordinate T>
Wide diameter_squared(const std::vector<BasicPoint<T>> &polygon) {
	auto result = convex_diameter(polygon);
	return result ? result->squared_distance : Wide(0);
}

template<Coordinate T>
std::optional<PointPair<T>> closest_pair(const std::vector<BasicPoint<T>> &points) {
	auto result = geometry::closest_pair(integer_geometry_detail::to_geometry(points));
	if (!result) return std::nullopt;
	return PointPair<T>{result->first, result->second, std::move(result->squared_distance)};
}

template<Coordinate T>
std::optional<PointPair<T>> furthest_pair(const std::vector<BasicPoint<T>> &points) {
	auto result = geometry::furthest_pair(integer_geometry_detail::to_geometry(points));
	if (!result) return std::nullopt;
	return PointPair<T>{result->first, result->second, std::move(result->squared_distance)};
}

template<Coordinate T>
std::vector<BasicPoint<T>> minkowski_sum(const std::vector<BasicPoint<T>> &a, const std::vector<BasicPoint<T>> &b) {
	return integer_geometry_detail::from_geometry(geometry::minkowski_sum(
		integer_geometry_detail::to_geometry(a), integer_geometry_detail::to_geometry(b)));
}
}

#endif
