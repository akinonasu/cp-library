#ifndef INTEGER_GEOMETRY_LATTICE_HPP
#define INTEGER_GEOMETRY_LATTICE_HPP

#include "point.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

namespace integer_geometry {
inline Wide gcd(Wide a, Wide b) {
	auto magnitude = [](Wide x) { return x < 0 ? __uint128_t(0) - __uint128_t(x) : __uint128_t(x); };
	auto u = magnitude(a), v = magnitude(b);
	while (v != 0) {
		auto remainder = u % v;
		u = v;
		v = remainder;
	}
	assert(u <= __uint128_t(std::numeric_limits<Wide>::max()));
	return Wide(u);
}

inline Point primitive_direction(const Point &direction) {
	Wide divisor = gcd(direction.x, direction.y);
	if (divisor == 0) return {};
	return {Integer(Wide(direction.x) / divisor), Integer(Wide(direction.y) / divisor)};
}

inline Wide segment_lattice_points(const Point &a, const Point &b) {
	return gcd(Wide(b.x) - a.x, Wide(b.y) - a.y) + 1;
}

inline Wide boundary_lattice_points(const std::vector<Point> &polygon) {
	if (polygon.empty()) return 0;
	std::size_t second = 1;
	while (second < polygon.size() && polygon[second] == polygon[0]) ++second;
	bool collinear = true;
	if (second < polygon.size())
		for (const auto &p : polygon)
			if (cross(polygon[0], polygon[second], p) != 0) { collinear = false; break; }
	if (collinear) {
		auto [first, last] = std::minmax_element(polygon.begin(), polygon.end());
		return segment_lattice_points(*first, *last);
	}
	Wide result = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		const auto &a = polygon[i], &b = polygon[(i + 1) % polygon.size()];
		result += gcd(Wide(b.x) - a.x, Wide(b.y) - a.y);
	}
	return result;
}

inline Wide interior_lattice_points(const std::vector<Point> &polygon) {
	if (polygon.size() < 3) return 0;
	Wide twice_area = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i)
		twice_area += cross(polygon[i], polygon[(i + 1) % polygon.size()]);
	if (twice_area < 0) twice_area = -twice_area;
	if (twice_area == 0) return 0;
	Wide twice_count = twice_area - boundary_lattice_points(polygon) + 2;
	assert(twice_count >= 0 && twice_count % 2 == 0);
	return twice_count / 2;
}
}

#endif
