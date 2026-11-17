#ifndef GEOMETRY_EXACT_POLYGON_HPP
#define GEOMETRY_EXACT_POLYGON_HPP

#include "exact_linear.hpp"
#include "../components/exact_geometry_integer.hpp"
#include "polygon.hpp"
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

namespace geometry {
template<ExactCoordinate T>
Rational area_exact(const std::vector<Point<T>> &polygon) {
	if constexpr (exact_integer_detail::coordinate<T>) {
		Integer total = 0;
		if (polygon.size() < 3) return 0;
		BigPoint origin = exact_integer_detail::convert(polygon[0]);
		for (std::size_t i = 2; i < polygon.size(); ++i)
			total += cross(origin, exact_integer_detail::convert(polygon[i - 1]), exact_integer_detail::convert(polygon[i]));
		return Rational(total < 0 ? -total : total) / 2;
	}
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
	if constexpr (exact_integer_detail::coordinate<T> && exact_integer_detail::coordinate<U>) {
		assert(line.a != line.b);
		std::vector<RPoint> result;
		if (polygon.empty()) return result;
		result.reserve(polygon.size() + 1);
		auto append = [&](RPoint point) { if (result.empty() || result.back() != point) result.push_back(std::move(point)); };
		BigPoint first = exact_integer_detail::convert(line.a), second = exact_integer_detail::convert(line.b);
		BigPoint a = exact_integer_detail::convert(polygon[0]);
		Integer ca = cross(first, second, a);
		for (std::size_t i = 0; i < polygon.size(); ++i) {
			BigPoint b = exact_integer_detail::convert(polygon[i + 1 == polygon.size() ? 0 : i + 1]);
			Integer cb = cross(first, second, b);
			int sa = sign(ca), sb = sign(cb);
			if (sa >= 0) append(to_rational(a));
			if (sa * sb < 0) {
				Integer denominator = ca - cb;
				append(exact_integer_detail::quotient(a * denominator + (b - a) * ca, denominator));
			}
			a = std::move(b);
			ca = std::move(cb);
		}
		if (result.size() > 1 && result.front() == result.back()) result.pop_back();
		return result;
	}

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
