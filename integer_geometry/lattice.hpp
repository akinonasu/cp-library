#ifndef INTEGER_GEOMETRY_LATTICE_HPP
#define INTEGER_GEOMETRY_LATTICE_HPP

#include "point.hpp"
#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

namespace integer_geometry {
inline Integer gcd(Integer a, Integer b) {
	if (a < 0) a = -a;
	if (b < 0) b = -b;
	while (b != 0) {
		Integer remainder = a % b;
		a = std::move(b);
		b = std::move(remainder);
	}
	return a;
}

inline Point primitive_direction(const Point &direction) {
	Integer divisor = gcd(direction.x, direction.y);
	if (divisor == 0) return {};
	return {direction.x / divisor, direction.y / divisor};
}

inline Integer segment_lattice_points(const Point &a, const Point &b) {
	return gcd(Integer(b.x - a.x), Integer(b.y - a.y)) + 1;
}

inline Integer boundary_lattice_points(const std::vector<Point> &polygon) {
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
	Integer result = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		const auto &a = polygon[i], &b = polygon[(i + 1) % polygon.size()];
		result += gcd(Integer(b.x - a.x), Integer(b.y - a.y));
	}
	return result;
}

inline Integer interior_lattice_points(const std::vector<Point> &polygon) {
	if (polygon.size() < 3) return 0;
	Integer twice_area = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i)
		twice_area += cross(polygon[i], polygon[(i + 1) % polygon.size()]);
	if (twice_area < 0) twice_area = -twice_area;
	if (twice_area == 0) return 0;
	Integer twice_count = twice_area - boundary_lattice_points(polygon) + 2;
	assert(twice_count >= 0 && twice_count % 2 == 0);
	return twice_count / 2;
}
}

#endif
