#ifndef GEOMETRY_LINEAR_HPP
#define GEOMETRY_LINEAR_HPP

#include "point.hpp"
#include <algorithm>
#include <array>
#include <vector>

namespace geometry {
template<class T> struct Line { Point<T> a, b; };
template<class T> struct Segment { Point<T> a, b; };

template<class T>
constexpr bool parallel(Line<T> l, Line<T> m, Real eps = epsilon) {
	using W = wide_t<T>;
	return sign((W(l.b.x) - l.a.x) * (W(m.b.y) - m.a.y) - (W(l.b.y) - l.a.y) * (W(m.b.x) - m.a.x), eps) == 0;
}
template<class T>
constexpr bool orthogonal(Line<T> l, Line<T> m, Real eps = epsilon) {
	using W = wide_t<T>;
	return sign((W(l.b.x) - l.a.x) * (W(m.b.x) - m.a.x) + (W(l.b.y) - l.a.y) * (W(m.b.y) - m.a.y), eps) == 0;
}
template<class T>
constexpr bool on_line(Line<T> l, Point<T> p, Real eps = epsilon) {
	return l.a == l.b ? almost_equal(l.a, p, eps) : orientation(l.a, l.b, p, eps) == 0;
}
template<class T>
constexpr bool on_segment(Segment<T> s, Point<T> p, Real eps = epsilon) {
	if (orientation(s.a, s.b, p, eps)) return false;
	if constexpr (integer<T>)
		return std::min(s.a.x, s.b.x) <= p.x && p.x <= std::max(s.a.x, s.b.x)
			&& std::min(s.a.y, s.b.y) <= p.y && p.y <= std::max(s.a.y, s.b.y);
	else return std::min(s.a.x, s.b.x) - eps <= p.x && p.x <= std::max(s.a.x, s.b.x) + eps
		&& std::min(s.a.y, s.b.y) - eps <= p.y && p.y <= std::max(s.a.y, s.b.y) + eps;
}
template<class T>
constexpr bool intersects(Segment<T> s, Segment<T> t, Real eps = epsilon) {
	int a = orientation(s.a, s.b, t.a, eps), b = orientation(s.a, s.b, t.b, eps);
	int c = orientation(t.a, t.b, s.a, eps), d = orientation(t.a, t.b, s.b, eps);
	return (!a && on_segment(s, t.a, eps)) || (!b && on_segment(s, t.b, eps))
		|| (!c && on_segment(t, s.a, eps)) || (!d && on_segment(t, s.b, eps)) || (a * b < 0 && c * d < 0);
}
template<class T>
constexpr bool intersects(Line<T> l, Segment<T> s, Real eps = epsilon) {
	if (l.a == l.b) return on_segment(s, l.a, eps);
	return orientation(l.a, l.b, s.a, eps) * orientation(l.a, l.b, s.b, eps) <= 0;
}
template<class T>
constexpr bool intersects(Segment<T> s, Line<T> l, Real eps = epsilon) { return intersects(l, s, eps); }
template<class T>
constexpr bool intersects(Line<T> l, Line<T> m, Real eps = epsilon) {
	if (l.a == l.b) return on_line(m, l.a, eps);
	if (m.a == m.b) return on_line(l, m.a, eps);
	return !parallel(l, m, eps) || on_line(l, m.a, eps);
}
template<class T>
FPoint projection(Line<T> l, Point<T> p) {
	using W = wide_t<T>;
	W x = W(l.b.x) - l.a.x, y = W(l.b.y) - l.a.y, n = x * x + y * y;
	if (!n) return to_real(l.a);
	Real t = Real(x * (W(p.x) - l.a.x) + y * (W(p.y) - l.a.y)) / Real(n);
	return {Real(l.a.x) + Real(x) * t, Real(l.a.y) + Real(y) * t};
}
template<class T>
FPoint reflection(Line<T> l, Point<T> p) { return projection(l, p) * Real(2) - to_real(p); }
template<class T>
std::optional<FPoint> intersection(Line<T> l, Line<T> m, Real eps = epsilon) {
	if (l.a == l.b) return on_line(m, l.a, eps) ? std::optional(to_real(l.a)) : std::nullopt;
	if (m.a == m.b) return on_line(l, m.a, eps) ? std::optional(to_real(m.a)) : std::nullopt;
	using W = wide_t<T>;
	W x = W(l.b.x) - l.a.x, y = W(l.b.y) - l.a.y, u = W(m.b.x) - m.a.x, v = W(m.b.y) - m.a.y;
	W d = x * v - y * u;
	if (!sign(d, eps)) return std::nullopt;
	Real t = Real((W(m.a.x) - l.a.x) * v - (W(m.a.y) - l.a.y) * u) / Real(d);
	return FPoint{Real(l.a.x) + Real(x) * t, Real(l.a.y) + Real(y) * t};
}
template<class T>
std::vector<FPoint> intersection(Segment<T> s, Segment<T> t, Real eps = epsilon) {
	if (!intersects(s, t, eps)) return {};
	if (auto p = intersection(Line<T>{s.a, s.b}, Line<T>{t.a, t.b}, eps)) return {*p};
	std::array<Point<T>, 4> candidates{s.a, s.b, t.a, t.b};
	std::sort(candidates.begin(), candidates.end());
	std::vector<FPoint> result;
	std::optional<Point<T>> first, last;
	for (auto p : candidates) if (on_segment(s, p, eps) && on_segment(t, p, eps)) {
		if (!first) first = p;
		last = p;
	}
	if (first) {
		result.push_back(to_real(*first));
		if (!almost_equal(*first, *last, eps)) result.push_back(to_real(*last));
	}
	return result;
}
template<class T>
std::vector<FPoint> intersection(Line<T> l, Segment<T> s, Real eps = epsilon) {
	if (!intersects(l, s, eps)) return {};
	if (auto p = intersection(l, Line<T>{s.a, s.b}, eps)) return {*p};
	if (almost_equal(s.a, s.b, eps)) return {to_real(s.a)};
	return {to_real(s.a), to_real(s.b)};
}
template<class T>
std::vector<FPoint> intersection(Segment<T> s, Line<T> l, Real eps = epsilon) { return intersection(l, s, eps); }
template<class T>
Real distance_squared(Line<T> l, Point<T> p) {
	if (l.a == l.b) return Real(distance_squared(l.a, p));
	Real c = Real(cross(l.a, l.b, p));
	return c * c / Real(distance_squared(l.a, l.b));
}
template<class T>
Real distance_squared(Segment<T> s, Point<T> p) {
	using W = wide_t<T>;
	W x = W(s.b.x) - s.a.x, y = W(s.b.y) - s.a.y;
	W n = x * x + y * y, d = (W(p.x) - s.a.x) * x + (W(p.y) - s.a.y) * y;
	if (d <= 0) return Real(distance_squared(s.a, p));
	if (d >= n) return Real(distance_squared(s.b, p));
	Real c = Real(cross(s.a, s.b, p));
	return c * c / Real(n);
}
template<class T> Real distance(Line<T> l, Point<T> p) { return std::sqrt(distance_squared(l, p)); }
template<class T> Real distance(Point<T> p, Line<T> l) { return distance(l, p); }
template<class T> Real distance(Segment<T> s, Point<T> p) { return std::sqrt(distance_squared(s, p)); }
template<class T> Real distance(Point<T> p, Segment<T> s) { return distance(s, p); }
template<class T> Real distance_squared(Point<T> p, Line<T> l) { return distance_squared(l, p); }
template<class T> Real distance_squared(Point<T> p, Segment<T> s) { return distance_squared(s, p); }
template<class T>
Real distance_squared(Line<T> l, Line<T> m, Real eps = epsilon) {
	if (intersects(l, m, eps)) return 0;
	return l.a == l.b ? distance_squared(m, l.a) : distance_squared(l, m.a);
}
template<class T>
Real distance_squared(Line<T> l, Segment<T> s, Real eps = epsilon) {
	if (intersects(l, s, eps)) return 0;
	return l.a == l.b ? distance_squared(s, l.a) : std::min(distance_squared(l, s.a), distance_squared(l, s.b));
}
template<class T>
Real distance_squared(Segment<T> s, Line<T> l, Real eps = epsilon) { return distance_squared(l, s, eps); }
template<class T>
Real distance_squared(Segment<T> s, Segment<T> t, Real eps = epsilon) {
	if (intersects(s, t, eps)) return 0;
	return std::min({distance_squared(s, t.a), distance_squared(s, t.b), distance_squared(t, s.a), distance_squared(t, s.b)});
}
template<class T> Real distance(Line<T> l, Line<T> m, Real eps = epsilon) { return std::sqrt(distance_squared(l, m, eps)); }
template<class T> Real distance(Line<T> l, Segment<T> s, Real eps = epsilon) { return std::sqrt(distance_squared(l, s, eps)); }
template<class T> Real distance(Segment<T> s, Line<T> l, Real eps = epsilon) { return distance(l, s, eps); }
template<class T> Real distance(Segment<T> s, Segment<T> t, Real eps = epsilon) { return std::sqrt(distance_squared(s, t, eps)); }
template<class T>
std::optional<Line<Real>> perpendicular_bisector(Point<T> a, Point<T> b) {
	if (a == b) return std::nullopt;
	using W = wide_t<T>;
	FPoint mid{Real(a.x) / 2 + Real(b.x) / 2, Real(a.y) / 2 + Real(b.y) / 2};
	FPoint d{Real(W(b.y) - a.y), Real(W(a.x) - b.x)};
	return Line<Real>{mid, mid + d};
}
}

#endif
