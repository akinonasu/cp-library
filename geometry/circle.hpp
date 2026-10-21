#ifndef GEOMETRY_CIRCLE_HPP
#define GEOMETRY_CIRCLE_HPP

#include "linear.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <optional>
#include <random>
#include <vector>

namespace geometry {
struct Circle { FPoint center{}; Real radius = 0; };
enum class CircleRelation { separate, external_tangent, intersecting, internal_tangent, contained, coincident };

namespace circle_detail {
inline void check([[maybe_unused]] const Circle &c, [[maybe_unused]] Real eps = 0) { assert(c.radius >= 0 && eps >= 0); }
inline FPoint perpendicular(FPoint p) { return {-p.y, p.x}; }
inline std::vector<FPoint> ordered(FPoint a, FPoint b) {
	if (b < a) std::swap(a, b);
	return {a, b};
}
inline Circle diameter(FPoint a, FPoint b) {
	FPoint center = a + (b - a) / Real(2);
	return {center, distance(center, a)};
}
inline Real segment_area(Real radius, Real along, Real height) {
	Real angle = std::atan2(height, along), square = angle * angle;
	Real value = angle < 0.001L ? angle * square * (Real(2) / 3 + square * (-Real(2) / 15 + square * (Real(4) / 315 - square * Real(2) / 2835)))
		: angle - std::sin(2 * angle) / 2;
	return radius * radius * value;
}
inline Real chord_position(Real radius, Real other, Real d) {
	if (std::abs(d - other) < std::abs(radius - other)) return ((d - other) * (d + other) + radius * radius) / (2 * d);
	return (d + (radius - other) * (radius + other) / d) / 2;
}
}

inline Real area(const Circle &c) { circle_detail::check(c); return std::numbers::pi_v<Real> * c.radius * c.radius; }
inline Real circumference(const Circle &c) { circle_detail::check(c); return 2 * std::numbers::pi_v<Real> * c.radius; }

template <class T>
Location contains(const Circle &c, Point<T> point, Real eps = epsilon) {
	circle_detail::check(c, eps);
	Real d2 = norm2(to_real(point) - c.center), outer = c.radius + eps;
	if (d2 > outer * outer) return Location::outside;
	Real inner = c.radius - eps;
	return inner > 0 && d2 < inner * inner ? Location::inside : Location::boundary;
}

inline CircleRelation relation(const Circle &a, const Circle &b, Real eps = epsilon) {
	circle_detail::check(a, eps);
	circle_detail::check(b);
	Real d = distance(a.center, b.center), small = std::min(a.radius, b.radius), large = std::max(a.radius, b.radius);
	if (d <= eps && large - small <= eps) return CircleRelation::coincident;
	Real gap = d - large;
	if (gap > small + eps) return CircleRelation::separate;
	if (gap < -small - eps) return CircleRelation::contained;
	if (std::abs(gap - small) <= eps) return CircleRelation::external_tangent;
	if (std::abs(gap + small) <= eps) return CircleRelation::internal_tangent;
	return CircleRelation::intersecting;
}

template <class T>
std::vector<FPoint> intersection(const Circle &c, const Line<T> &line, Real eps = epsilon) {
	circle_detail::check(c, eps);
	FPoint a = to_real(line.a), d = to_real(line.b) - a;
	Real length2 = norm2(d);
	if (!length2) return contains(c, a, eps) == Location::boundary ? std::vector<FPoint>{a} : std::vector<FPoint>{};
	FPoint middle = a + d * (dot(c.center - a, d) / length2);
	Real height = distance(c.center, middle);
	if (height > c.radius + eps) return {};
	if (std::abs(height - c.radius) <= eps) return {middle};
	FPoint offset = d * std::sqrt(std::max(Real(0), (c.radius - height) * (c.radius + height) / length2));
	return circle_detail::ordered(middle - offset, middle + offset);
}

template <class T>
std::vector<FPoint> intersection(const Circle &c, const Segment<T> &segment, Real eps = epsilon) {
	auto points = intersection(c, Line<T>{segment.a, segment.b}, eps);
	FPoint a = to_real(segment.a), d = to_real(segment.b) - a;
	Real length2 = norm2(d);
	if (!length2) return points;
	Real tolerance = eps / std::sqrt(length2);
	std::erase_if(points, [&](FPoint p) {
		Real t = dot(p - a, d) / length2;
		return t < -tolerance || t > 1 + tolerance;
	});
	return points;
}

inline std::optional<std::vector<FPoint>> intersection(const Circle &a, const Circle &b, Real eps = epsilon) {
	CircleRelation type = relation(a, b, eps);
	if (type == CircleRelation::coincident) {
		if (a.radius == 0 && b.radius == 0) return std::vector<FPoint>{a.center + (b.center - a.center) / Real(2)};
		return std::nullopt;
	}
	if (type == CircleRelation::separate || type == CircleRelation::contained) return std::vector<FPoint>{};
	const Circle &first = a.radius <= b.radius ? a : b, &second = a.radius <= b.radius ? b : a;
	FPoint v = second.center - first.center;
	Real d = std::sqrt(norm2(v));
	Real along = circle_detail::chord_position(first.radius, second.radius, d);
	FPoint middle = first.center + v * (along / d);
	if (type != CircleRelation::intersecting) return std::vector<FPoint>{middle};
	Real height2 = std::max(Real(0), (first.radius - along) * (first.radius + along));
	if (!height2) return std::vector<FPoint>{middle};
	FPoint offset = circle_detail::perpendicular(v) * (std::sqrt(height2) / d);
	return circle_detail::ordered(middle - offset, middle + offset);
}

template <class T>
std::vector<FPoint> tangent_points(const Circle &c, Point<T> point, Real eps = epsilon) {
	circle_detail::check(c, eps);
	FPoint p = to_real(point), v = p - c.center;
	Real d2 = norm2(v), d = std::sqrt(d2);
	if (!d) return c.radius == 0 ? std::vector<FPoint>{c.center} : std::vector<FPoint>{};
	if (d < c.radius - eps) return {};
	if (c.radius == 0) return {c.center};
	if (std::abs(d - c.radius) <= eps) return {c.center + v * (c.radius / d)};
	FPoint middle = c.center + v * (c.radius * c.radius / d2);
	FPoint offset = circle_detail::perpendicular(v) * (c.radius * std::sqrt(std::max(Real(0), (d - c.radius) * (d + c.radius))) / d2);
	return circle_detail::ordered(middle - offset, middle + offset);
}

inline std::optional<std::vector<Line<Real>>> common_tangents(const Circle &a, const Circle &b, Real eps = epsilon) {
	circle_detail::check(a, eps);
	circle_detail::check(b);
	FPoint v = b.center - a.center;
	Real d2 = norm2(v), d = std::sqrt(d2);
	if (d <= eps) {
		if (std::abs(a.radius - b.radius) <= eps) return std::nullopt;
		return std::vector<Line<Real>>{};
	}
	std::vector<Line<Real>> result;
	result.reserve(4);
	Real scale = std::max({Real(1), d, a.radius, b.radius});
	if (!a.radius && !b.radius) return std::vector<Line<Real>>{Line<Real>{a.center, b.center}};
	if (!a.radius || !b.radius) {
		const Circle &c = a.radius ? a : b;
		FPoint p = a.radius ? b.center : a.center;
		for (FPoint q : tangent_points(c, p, eps))
			result.push_back({q, q + circle_detail::perpendicular((q - c.center) / c.radius) * scale});
		return result;
	}
	for (int side : {1, -1}) {
		Real difference = a.radius - side * b.radius, absolute = std::abs(difference);
		if (d < absolute - eps) continue;
		Real height = std::abs(d - absolute) <= eps ? 0 : std::sqrt(std::max(Real(0), (d - absolute) * (d + absolute)));
		for (int direction : {1, -1}) {
			FPoint normal = (v * difference + circle_detail::perpendicular(v) * (height * direction)) / d2;
			FPoint p = a.center + normal * a.radius;
			result.push_back({p, p + circle_detail::perpendicular(normal) * scale});
			if (!height) break;
		}
	}
	return result;
}

template <class T>
std::optional<Circle> circumcircle(Point<T> pa, Point<T> pb, Point<T> pc, Real eps = epsilon) {
	assert(eps >= 0);
	FPoint a = to_real(pa), u = to_real(pb) - a, v = to_real(pc) - a;
	Real determinant = cross(u, v), u2 = norm2(u), v2 = norm2(v);
	if (std::abs(determinant) <= eps * std::sqrt(std::max(u2, v2))) return std::nullopt;
	FPoint offset{(u2 * v.y - v2 * u.y) / (2 * determinant), (u.x * v2 - v.x * u2) / (2 * determinant)};
	return Circle{a + offset, std::sqrt(norm2(offset))};
}

template <class T>
std::optional<Circle> incircle(Point<T> pa, Point<T> pb, Point<T> pc, Real eps = epsilon) {
	assert(eps >= 0);
	FPoint a = to_real(pa), b = to_real(pb), c = to_real(pc);
	Real x = distance(b, c), y = distance(c, a), z = distance(a, b), total = x + y + z;
	Real area2 = std::abs(cross(b - a, c - a));
	if (!total || area2 <= eps * std::max({x, y, z})) return std::nullopt;
	return Circle{a + ((b - a) * y + (c - a) * z) / total, area2 / total};
}

inline Real overlap_area(const Circle &a, const Circle &b) {
	circle_detail::check(a);
	circle_detail::check(b);
	Real d = distance(a.center, b.center), r = std::min(a.radius, b.radius), large = std::max(a.radius, b.radius), gap = d - large;
	if (!r || gap >= r) return 0;
	if (gap <= -r) return std::numbers::pi_v<Real> * r * r;
	Real x = circle_detail::chord_position(r, large, d);
	Real height = std::sqrt(std::max(Real(0), (r - x) * (r + x)));
	return std::clamp(circle_detail::segment_area(r, x, height) + circle_detail::segment_area(large, d - x, height), Real(0), std::numbers::pi_v<Real> * r * r);
}

template <class T>
Real intersection_area(const Circle &c, const std::vector<Point<T>> &polygon) {
	circle_detail::check(c);
	if (!c.radius || polygon.size() < 3) return 0;
	Real result = 0, radius2 = c.radius * c.radius;
	for (std::size_t i = 0; i < polygon.size(); ++i) {
		FPoint p = to_real(polygon[i]) - c.center, q = to_real(polygon[(i + 1) % polygon.size()]) - c.center, d = q - p;
		Real a = norm2(d);
		if (!a) continue;
		if (norm2(p) <= radius2 && norm2(q) <= radius2) { result += cross(p, q); continue; }
		Real b = dot(p, d), perpendicular = cross(p, d), determinant = a * radius2 - perpendicular * perpendicular;
		std::array<Real, 4> split{0};
		int count = 1;
		if (determinant > 0) {
			Real root = std::sqrt(determinant);
			for (Real t : {(-b - root) / a, (-b + root) / a}) if (0 < t && t < 1) split[count++] = t;
		}
		split[count++] = 1;
		for (int j = 1; j < count; ++j) {
			FPoint u = p + d * split[j - 1], v = p + d * split[j];
			if (norm2(p + d * ((split[j - 1] + split[j]) / 2)) < radius2) result += cross(u, v);
			else result += radius2 * std::atan2(cross(u, v), dot(u, v));
		}
	}
	return std::abs(result) / 2;
}

template <class T>
std::optional<Circle> minimum_enclosing_circle(const std::vector<Point<T>> &points, std::uint64_t seed = 712367821, Real eps = epsilon) {
	assert(eps >= 0);
	if (points.empty()) return std::nullopt;
	std::vector<FPoint> p;
	p.reserve(points.size());
	for (auto point : points) p.push_back(to_real(point));
	std::mt19937_64 random(seed);
	std::shuffle(p.begin(), p.end(), random);
	Circle result{p[0], 0};
	for (std::size_t i = 1; i < p.size(); ++i) if (contains(result, p[i], eps) == Location::outside) {
		result = {p[i], 0};
		for (std::size_t j = 0; j < i; ++j) if (contains(result, p[j], eps) == Location::outside) {
			result = circle_detail::diameter(p[i], p[j]);
			for (std::size_t k = 0; k < j; ++k) if (contains(result, p[k], eps) == Location::outside) {
				auto c = circumcircle(p[i], p[j], p[k], Real(0));
				if (c) result = *c;
				else {
					std::array<Circle, 3> candidates{circle_detail::diameter(p[i], p[j]), circle_detail::diameter(p[i], p[k]), circle_detail::diameter(p[j], p[k])};
					result = *std::max_element(candidates.begin(), candidates.end(), [](const Circle &a, const Circle &b) { return a.radius < b.radius; });
				}
			}
		}
	}
	Real radius2 = result.radius * result.radius;
	for (FPoint point : p) radius2 = std::max(radius2, norm2(point - result.center));
	result.radius = std::sqrt(radius2);
	return result;
}
}

#endif
