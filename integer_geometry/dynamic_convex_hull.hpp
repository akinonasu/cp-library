#ifndef INTEGER_GEOMETRY_DYNAMIC_CONVEX_HULL_HPP
#define INTEGER_GEOMETRY_DYNAMIC_CONVEX_HULL_HPP

#include "../components/integer_geometry_bridge.hpp"
#include "../geometry/dynamic_convex_hull.hpp"
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace integer_geometry {
template<Coordinate T = Integer>
class DynamicConvexHull {
	geometry::DynamicConvexHull<T> hull_;
public:
	using point_type = BasicPoint<T>;
	using value_type = Wide;
	DynamicConvexHull() = default;
	explicit DynamicConvexHull(const std::vector<point_type> &points) { for (auto p : points) insert(p); }
	DynamicConvexHull(const DynamicConvexHull &) = default;
	DynamicConvexHull(DynamicConvexHull &&) noexcept = default;
	DynamicConvexHull &operator=(DynamicConvexHull other) noexcept { swap(other); return *this; }
	void swap(DynamicConvexHull &other) noexcept { hull_.swap(other.hull_); }
	friend void swap(DynamicConvexHull &a, DynamicConvexHull &b) noexcept { a.swap(b); }
	bool empty() const { return hull_.empty(); }
	std::size_t size() const { return hull_.size(); }
	void clear() { hull_.clear(); }
	bool insert(point_type point) { return hull_.insert(integer_geometry_detail::to_geometry(point)); }
	std::vector<point_type> vertices() const { return integer_geometry_detail::from_geometry(hull_.vertices()); }
	Wide doubled_area() const { return hull_.doubled_area(); }
	Location contains(point_type point) const {
		return integer_geometry_detail::from_geometry(hull_.contains(integer_geometry_detail::to_geometry(point)));
	}
	std::optional<point_type> support(point_type direction) const {
		auto point = hull_.support(integer_geometry_detail::to_geometry(direction));
		if (!point) return std::nullopt;
		return integer_geometry_detail::from_geometry(*point);
	}
	std::optional<Wide> max_dot(point_type direction) const { return hull_.max_dot(integer_geometry_detail::to_geometry(direction)); }
	std::optional<Wide> min_dot(point_type direction) const { return hull_.min_dot(integer_geometry_detail::to_geometry(direction)); }
};
}

#endif
