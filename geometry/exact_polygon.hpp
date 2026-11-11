#ifndef GEOMETRY_EXACT_POLYGON_HPP
#define GEOMETRY_EXACT_POLYGON_HPP

#include "exact_linear.hpp"
#include "polygon.hpp"
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

namespace geometry {
template<ExactCoordinate T>
Rational area_exact(const std::vector<Point<T>> &polygon) {
	Rational total = 0;
	if (polygon.size() < 3) return total;
	RPoint origin = to_rational(polygon[0]);
	for (std::size_t i = 2; i < polygon.size(); ++i)
		total += cross(origin, to_rational(polygon[i - 1]), to_rational(polygon[i]));
	return (total < 0 ? -total : total) / 2;
}

template<ExactCoordinate T>
std::optional<RPoint> centroid_exact(const std::vector<Point<T>> &polygon) {
	if (polygon.size() < 3) return std::nullopt;
	RPoint origin = to_rational(polygon[0]), sum{};
	Rational total = 0;
	for (std::size_t i = 2; i < polygon.size(); ++i) {
		RPoint a = to_rational(polygon[i - 1]) - origin, b = to_rational(polygon[i]) - origin;
		Rational weight = cross(a, b);
		sum += (a + b) * weight;
		total += weight;
	}
	if (total == 0) return std::nullopt;
	return origin + sum / Rational(3 * total);
}

template<ExactCoordinate T, ExactCoordinate U>
std::vector<RPoint> convex_cut_exact(const std::vector<Point<T>> &polygon, Line<U> line) {
	assert(line.a != line.b);
	RPoint first = to_rational(line.a), second = to_rational(line.b);
	std::vector<RPoint> result;
	result.reserve(polygon.size() + 1);
	auto append = [&](RPoint p) { if (result.empty() || result.back() != p) result.push_back(std::move(p)); };
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		RPoint a = to_rational(polygon[i]), b = to_rational(polygon[(i + 1) % polygon.size()]);
		Rational ca = cross(first, second, a), cb = cross(first, second, b);
		int sa = sign(ca), sb = sign(cb);
		if (sa >= 0) append(a);
		if (sa * sb < 0) append(a + (b - a) * Rational(ca / (ca - cb)));
	}
	if (result.size() > 1 && result.front() == result.back()) result.pop_back();
	return result;
}
}

#endif
