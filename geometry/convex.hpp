#ifndef GEOMETRY_CONVEX_HPP
#define GEOMETRY_CONVEX_HPP

#include "polygon.hpp"
#include "../components/closest_pair.hpp"
#include "../components/packed_convex_hull.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <optional>
#include <vector>

namespace geometry {
template <class T>
struct PointPair {
	int first, second;
	wide_t<T> squared_distance;
};

template <class T>
std::vector<int> convex_hull_indices(const std::vector<Point<T>> &points, bool keep_collinear = false, Real eps = epsilon) {
	assert(points.size() <= std::size_t(std::numeric_limits<int>::max()));
	if constexpr (integer<T> && sizeof(T) <= 8) {
		if (points.size() >= 64) {
			T min_x = points[0].x, max_x = min_x, min_y = points[0].y, max_y = min_y;
			for (auto p : points) {
				min_x = std::min(min_x, p.x), max_x = std::max(max_x, p.x);
				min_y = std::min(min_y, p.y), max_y = std::max(max_y, p.y);
			}
			if (wide_t<T>(max_x) - min_x <= INT32_MAX && wide_t<T>(max_y) - min_y <= INT32_MAX) {
				std::vector<packed_convex_hull_detail::Entry> order(points.size());
				for (int i = 0; i < int(points.size()); ++i)
					order[i] = {(std::uint64_t(wide_t<T>(points[i].x) - min_x) << 32) | std::uint32_t(wide_t<T>(points[i].y) - min_y), i};
				auto hull = packed_convex_hull_detail::build(std::move(order), keep_collinear);
				std::vector<int> result;
				result.reserve(hull.size());
				for (auto value : hull) result.push_back(value.index);
				return result;
			}
		}
	}
	std::vector<int> order(points.size());
	std::iota(order.begin(), order.end(), 0);
	std::sort(order.begin(), order.end(), [&](int a, int b) { return points[a] < points[b]; });
	order.erase(std::unique(order.begin(), order.end(), [&](int a, int b) { return points[a] == points[b]; }), order.end());
	if (order.size() <= 2) return order;
	if (keep_collinear && std::all_of(order.begin() + 2, order.end(), [&](int i) {
		return !orientation(points[order[0]], points[order[1]], points[i], eps);
	})) return order;
	std::vector<int> hull;
	hull.reserve(order.size() * 2);
	auto append = [&](int i, std::size_t base) {
		while (hull.size() >= base) {
			int turn = orientation(points[hull[hull.size() - 2]], points[hull.back()], points[i], eps);
			if (turn > 0 || (keep_collinear && !turn)) break;
			hull.pop_back();
		}
		hull.push_back(i);
	};
	for (int i : order) append(i, 2);
	std::size_t base = hull.size() + 1;
	for (std::size_t i = order.size() - 1; i-- > 0;) append(order[i], base);
	hull.pop_back();
	return hull;
}

template <class T>
std::vector<Point<T>> convex_hull(std::vector<Point<T>> &&points, bool keep_collinear = false, Real eps = epsilon) {
	assert(points.size() <= std::size_t(std::numeric_limits<int>::max()));
	if constexpr (integer<T> && sizeof(T) <= 8) {
		if (points.size() >= 64) {
			T min_x = points[0].x, max_x = min_x, min_y = points[0].y, max_y = min_y;
			for (auto p : points) {
				min_x = std::min(min_x, p.x), max_x = std::max(max_x, p.x);
				min_y = std::min(min_y, p.y), max_y = std::max(max_y, p.y);
			}
			if (wide_t<T>(max_x) - min_x <= INT32_MAX && wide_t<T>(max_y) - min_y <= INT32_MAX) {
				std::vector<std::uint64_t> order(points.size());
				for (int i = 0; i < int(points.size()); ++i)
					order[i] = (std::uint64_t(wide_t<T>(points[i].x) - min_x) << 32) | std::uint32_t(wide_t<T>(points[i].y) - min_y);
				auto hull = packed_convex_hull_detail::build(std::move(order), keep_collinear);
				points.clear();
				for (auto value : hull) points.emplace_back(T(wide_t<T>(min_x) + (value >> 32)), T(wide_t<T>(min_y) + std::uint32_t(value)));
				return std::move(points);
			}
		}
	}
	std::sort(points.begin(), points.end());
	points.erase(std::unique(points.begin(), points.end()), points.end());
	if (points.size() <= 2 || (keep_collinear && std::all_of(points.begin() + 2, points.end(), [&](auto p) {
		return !orientation(points[0], points[1], p, eps);
	}))) return std::move(points);
	std::vector<Point<T>> hull;
	hull.reserve(points.size() * 2);
	auto append = [&](Point<T> p, std::size_t base) {
		while (hull.size() >= base) {
			int turn = orientation(hull[hull.size() - 2], hull.back(), p, eps);
			if (turn > 0 || (keep_collinear && !turn)) break;
			hull.pop_back();
		}
		hull.push_back(p);
	};
	for (auto p : points) append(p, 2);
	std::size_t base = hull.size() + 1;
	for (std::size_t i = points.size() - 1; i-- > 0;) append(points[i], base);
	hull.pop_back();
	return hull;
}

template <class T>
std::vector<Point<T>> convex_hull(const std::vector<Point<T>> &points, bool keep_collinear = false, Real eps = epsilon) {
	return convex_hull(std::vector<Point<T>>(points), keep_collinear, eps);
}

template <class T>
Location convex_contains(const std::vector<Point<T>> &polygon, Point<T> p, Real eps = epsilon) {
	if (polygon.empty()) return Location::outside;
	if (polygon.size() <= 2) return on_segment(Segment<T>{polygon.front(), polygon.back()}, p, eps) ? Location::boundary : Location::outside;
	int first = orientation(polygon[0], polygon[1], p, eps), last = orientation(polygon[0], polygon.back(), p, eps);
	if (first < 0 || last > 0) return Location::outside;
	if (!first) return on_segment(Segment<T>{polygon[0], polygon[1]}, p, eps) ? Location::boundary : Location::outside;
	if (!last) return on_segment(Segment<T>{polygon[0], polygon.back()}, p, eps) ? Location::boundary : Location::outside;
	std::size_t l = 1, r = polygon.size() - 1;
	while (r - l > 1) {
		std::size_t m = (l + r) / 2;
		if (orientation(polygon[0], polygon[m], p, eps) >= 0) l = m;
		else r = m;
	}
	int side = orientation(polygon[l], polygon[r], p, eps);
	if (side > 0) return Location::inside;
	return !side && on_segment(Segment<T>{polygon[l], polygon[r]}, p, eps) ? Location::boundary : Location::outside;
}

template <class T>
std::optional<PointPair<T>> convex_diameter(const std::vector<Point<T>> &polygon, Real eps = epsilon) {
	assert(polygon.size() <= std::size_t(std::numeric_limits<int>::max()));
	int n = int(polygon.size());
	if (!n) return std::nullopt;
	if (n == 1) return PointPair<T>{0, 0, 0};
	PointPair<T> result{0, 1, distance_squared(polygon[0], polygon[1])};
	auto update = [&](int a, int b) {
		auto d = distance_squared(polygon[a], polygon[b]);
		if (d > result.squared_distance) result = {a, b, d};
	};
	for (int i = 0, j = 1; i < n; ++i) {
		int next = (i + 1) % n;
		while (sign(cross(polygon[i], polygon[next], polygon[(j + 1) % n]) - cross(polygon[i], polygon[next], polygon[j]), eps) > 0) j = (j + 1) % n;
		update(i, j);
		update(next, j);
	}
	return result;
}

template <class T>
wide_t<T> diameter_squared(const std::vector<Point<T>> &polygon, Real eps = epsilon) {
	auto result = convex_diameter(polygon, eps);
	return result ? result->squared_distance : wide_t<T>(0);
}

template <class T>
std::optional<PointPair<T>> furthest_pair(const std::vector<Point<T>> &points, Real eps = epsilon) {
	if (points.size() < 2) return std::nullopt;
	auto polygon = convex_hull(points, false, eps);
	if (polygon.size() == 1) return PointPair<T>{0, 1, 0};
	auto result = *convex_diameter(polygon, eps);
	auto a = polygon[result.first], b = polygon[result.second];
	result.first = result.second = -1;
	for (int i = 0; i < int(points.size()); ++i) {
		if (points[i] == a) result.first = i;
		if (points[i] == b) result.second = i;
		if (result.first >= 0 && result.second >= 0) break;
	}
	return result;
}

template <class T>
std::vector<Point<T>> minkowski_sum(std::vector<Point<T>> a, std::vector<Point<T>> b) {
	auto normalize = [&](std::vector<Point<T>> &p) {
		p.erase(std::unique(p.begin(), p.end()), p.end());
		if (p.size() > 1 && p.front() == p.back()) p.pop_back();
		if (p.size() <= 1) return;
		int direction = sign(signed_area2(p), 0);
		if (!direction) {
			auto [lo, hi] = std::minmax_element(p.begin(), p.end());
			Point<T> first = *lo, last = *hi;
			p = {first};
			if (first != last) p.push_back(last);
		} else {
			if (direction < 0) std::reverse(p.begin(), p.end());
			std::vector<Point<T>> strict;
			strict.reserve(p.size());
			for (std::size_t i = 0; i < p.size(); ++i)
				if (orientation(p[(i + p.size() - 1) % p.size()], p[i], p[(i + 1) % p.size()], 0)) strict.push_back(p[i]);
			p = std::move(strict);
		}
		if (!p.empty()) std::rotate(p.begin(), std::min_element(p.begin(), p.end(), [](auto x, auto y) { return x.y != y.y ? x.y < y.y : x.x < y.x; }), p.end());
	};
	normalize(a), normalize(b);
	if (a.empty() || b.empty()) return {};
	std::vector<Point<T>> result;
	result.reserve(a.size() + b.size());
	for (std::size_t i = 0, j = 0; i < a.size() || j < b.size();) {
		result.push_back(a[i % a.size()] + b[j % b.size()]);
		if (i == a.size() || a.size() == 1) { ++j; i = a.size(); continue; }
		if (j == b.size() || b.size() == 1) { ++i; j = b.size(); continue; }
		const auto &p = a[i], &q = a[(i + 1) % a.size()], &r = b[j], &s = b[(j + 1) % b.size()];
		wide_t<T> ax = wide_t<T>(q.x) - p.x, ay = wide_t<T>(q.y) - p.y, bx = wide_t<T>(s.x) - r.x, by = wide_t<T>(s.y) - r.y;
		bool ha = ay < 0 || (ay == 0 && ax < 0), hb = by < 0 || (by == 0 && bx < 0);
		int turn = ha != hb ? (ha ? -1 : 1) : sign(ax * by - ay * bx, 0);
		if (turn >= 0) ++i;
		if (turn <= 0) ++j;
	}
	if (result.size() > 1 && result.front() == result.back()) result.pop_back();
	std::rotate(result.begin(), std::min_element(result.begin(), result.end()), result.end());
	return result;
}

template <class T>
std::optional<PointPair<T>> closest_pair(const std::vector<Point<T>> &points) {
	assert(points.size() <= std::size_t(std::numeric_limits<int>::max()));
	int n = int(points.size());
	if (n < 2) return std::nullopt;
	if (n <= 32) {
		PointPair<T> result{0, 1, distance_squared(points[0], points[1])};
		for (int i = 1; i < n; ++i) for (int j = 0; j < i; ++j) {
			auto d = distance_squared(points[i], points[j]);
			if (d < result.squared_distance) result = {i, j, d};
		}
		return result;
	}
	if constexpr (integer<T> && sizeof(T) <= 8) {
		if (n >= 64) {
			T min_x = points[0].x, max_x = min_x, min_y = points[0].y, max_y = min_y;
			for (auto p : points) {
				min_x = std::min(min_x, p.x), max_x = std::max(max_x, p.x);
				min_y = std::min(min_y, p.y), max_y = std::max(max_y, p.y);
			}
			if (wide_t<T>(max_x) - min_x <= INT32_MAX && wide_t<T>(max_y) - min_y <= INT32_MAX) {
				auto pack = [&](Point<T> p) {
					return (std::uint64_t(wide_t<T>(p.y) - min_y) << 32) | std::uint32_t(wide_t<T>(p.x) - min_x);
				};
				std::vector<std::uint64_t> packed(n);
				for (int i = 0; i < n; ++i) packed[i] = pack(points[i]);
				if (auto pair = closest_pair_detail::packed(std::move(packed), std::uint32_t(wide_t<T>(max_x) - min_x), std::uint64_t(n) * 32)) {
					int a = -1, b = -1;
					for (int i = 0; i < n && (a < 0 || b < 0); ++i) {
						auto value = pack(points[i]);
						if (value == pair->first && a < 0) a = i;
						else if (value == pair->second && b < 0) b = i;
					}
					return PointPair<T>{a, b, distance_squared(points[a], points[b])};
				}
			}
		}
	}
	struct Entry { Point<T> point; int index; };
	std::vector<Entry> sorted, buffer(n);
	sorted.reserve(n);
	for (int i = 0; i < n; ++i) sorted.push_back({points[i], i});
	std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) { return a.point < b.point; });
	for (int i = 1; i < n; ++i)
		if (sorted[i - 1].point == sorted[i].point) return PointPair<T>{sorted[i - 1].index, sorted[i].index, 0};
	PointPair<T> result{sorted[0].index, sorted[1].index, distance_squared(sorted[0].point, sorted[1].point)};
	auto update = [&](const Entry &a, const Entry &b) {
		auto d = distance_squared(a.point, b.point);
		if (d < result.squared_distance) result = {a.index, b.index, d};
	};
	auto by_y = [](const auto &a, const auto &b) { return a.point.y != b.point.y ? a.point.y < b.point.y : a.point.x < b.point.x; };
	auto solve = [&](auto &&self, int l, int r) -> void {
		if (r - l <= 3) {
			for (int i = l; i < r; ++i) for (int j = i + 1; j < r; ++j) update(sorted[i], sorted[j]);
			std::sort(sorted.begin() + l, sorted.begin() + r, by_y);
			return;
		}
		int m = (l + r) / 2;
		T middle_x = sorted[m].point.x;
		self(self, l, m), self(self, m, r);
		std::merge(sorted.begin() + l, sorted.begin() + m, sorted.begin() + m, sorted.begin() + r, buffer.begin() + l, by_y);
		std::copy(buffer.begin() + l, buffer.begin() + r, sorted.begin() + l);
		int count = 0;
		for (int i = l; i < r; ++i) {
			wide_t<T> dx = wide_t<T>(sorted[i].point.x) - middle_x;
			if (dx * dx >= result.squared_distance) continue;
			for (int j = count; j-- > 0;) {
				wide_t<T> dy = wide_t<T>(sorted[i].point.y) - buffer[l + j].point.y;
				if (dy * dy >= result.squared_distance) break;
				update(sorted[i], buffer[l + j]);
			}
			buffer[l + count++] = sorted[i];
		}
	};
	solve(solve, 0, n);
	return result;
}
}

#endif
