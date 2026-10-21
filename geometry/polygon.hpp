#ifndef GEOMETRY_POLYGON_HPP
#define GEOMETRY_POLYGON_HPP

#include "linear.hpp"
#include <cassert>
#include <cmath>
#include <optional>
#include <vector>

namespace geometry {
template <class T>
wide_t<T> signed_area2(const std::vector<Point<T>> &polygon) {
	wide_t<T> result = 0;
	for (std::size_t i = 2; i < polygon.size(); ++i) result += cross(polygon[0], polygon[i - 1], polygon[i]);
	return result;
}

template <class T>
Real area(const std::vector<Point<T>> &polygon) { return std::abs(Real(signed_area2(polygon))) / 2; }

template <class T>
Real perimeter(const std::vector<Point<T>> &polygon) {
	Real result = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) result += distance(polygon[i], polygon[(i + 1) % polygon.size()]);
	return result;
}

template <class T>
bool is_convex(const std::vector<Point<T>> &polygon, bool strict = false, Real eps = epsilon) {
	if (polygon.size() < 3) return !strict;
	int direction = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		int turn = orientation(polygon[i], polygon[(i + 1) % polygon.size()], polygon[(i + 2) % polygon.size()], eps);
		if (!turn) { if (strict) return false; }
		else if (direction && turn != direction) return false;
		else direction = turn;
	}
	return true;
}

template <class T>
Location contains(const std::vector<Point<T>> &polygon, Point<T> p, Real eps = epsilon) {
	int winding = 0;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		const auto &a = polygon[i], &b = polygon[(i + 1) % polygon.size()];
		if (on_segment(Segment<T>{a, b}, p, eps)) return Location::boundary;
		int ay = sign(wide_t<T>(a.y) - p.y, eps), by = sign(wide_t<T>(b.y) - p.y, eps);
		int turn = orientation(a, b, p, eps);
		if (ay <= 0 && by > 0 && turn > 0) ++winding;
		if (ay > 0 && by <= 0 && turn < 0) --winding;
	}
	return winding ? Location::inside : Location::outside;
}

template <class T>
std::optional<FPoint> centroid(const std::vector<Point<T>> &polygon, Real eps = epsilon) {
	if (polygon.size() < 3) return std::nullopt;
	using W = wide_t<T>;
	FPoint origin = to_real(polygon[0]), sum{};
	W total = 0;
	for (std::size_t i = 2; i < polygon.size(); ++i) {
		FPoint a{Real(W(polygon[i - 1].x) - polygon[0].x), Real(W(polygon[i - 1].y) - polygon[0].y)};
		FPoint b{Real(W(polygon[i].x) - polygon[0].x), Real(W(polygon[i].y) - polygon[0].y)};
		W weight = cross(polygon[0], polygon[i - 1], polygon[i]);
		sum += (a + b) * Real(weight);
		total += weight;
	}
	if (!sign(total, eps)) return std::nullopt;
	return origin + sum / (3 * Real(total));
}

template <class T>
std::vector<FPoint> convex_cut(const std::vector<Point<T>> &polygon, Line<T> line, Real eps = epsilon) {
	assert(line.a != line.b);
	std::vector<FPoint> result;
	result.reserve(polygon.size() + 1);
	auto append = [&](FPoint p) { if (result.empty() || result.back() != p) result.push_back(p); };
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		const auto &a = polygon[i], &b = polygon[(i + 1) % polygon.size()];
		auto ca = cross(line.a, line.b, a), cb = cross(line.a, line.b, b);
		int sa = sign(ca, eps), sb = sign(cb, eps);
		if (sa >= 0) append(to_real(a));
		if (sa * sb < 0) append(to_real(a) + (to_real(b) - to_real(a)) * (Real(ca) / (Real(ca) - Real(cb))));
	}
	if (result.size() > 1 && result.front() == result.back()) result.pop_back();
	return result;
}
}

#endif
