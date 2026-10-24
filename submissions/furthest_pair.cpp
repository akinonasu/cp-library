#line 1 "test/geometry_furthest_pair.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/furthest_pair"
#line 1 "geometry/convex.hpp"



#line 1 "geometry/polygon.hpp"



#line 1 "geometry/linear.hpp"



#line 1 "geometry/point.hpp"



#line 1 "components/radix_sort.hpp"



#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

template <int Bits, class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key &&key) {
	static_assert(1 <= Bits && Bits <= 16);
	constexpr unsigned buckets = 1U << Bits, mask = buckets - 1;
	using K = std::remove_cvref_t<decltype(key(std::declval<const T &>()))>;
	static_assert(std::is_integral_v<K> && std::is_unsigned_v<K> && sizeof(K) <= sizeof(std::uint64_t));
	if (values.size() < 2) return;
	if (values.size() < 256) {
		std::stable_sort(values.begin(), values.end(), [&](const T &a, const T &b) { return key(a) < key(b); });
		return;
	}
	std::uint64_t first = key(values.front()), previous = first, varying = 0;
	bool sorted = true;
	for (std::size_t i = 1; i < values.size(); ++i) {
		std::uint64_t current = key(values[i]);
		varying |= first ^ current;
		sorted &= previous <= current;
		previous = current;
	}
	if (sorted) return;
	std::vector<T> scratch(values.size());
	using Counts = std::conditional_t<(Bits <= 11), std::array<std::size_t, buckets>, std::vector<std::size_t>>;
	Counts count = [] {
		if constexpr (Bits <= 11) return Counts{};
		else return Counts(buckets);
	}();
	for (int shift = 0; shift < 64; shift += Bits) {
		if (!((varying >> shift) & mask)) continue;
		std::fill(count.begin(), count.end(), 0);
		for (const auto &value : values) ++count[(static_cast<std::uint64_t>(key(value)) >> shift) & mask];
		std::size_t sum = 0;
		for (auto &x : count) {
			auto n = x;
			x = sum, sum += n;
		}
		for (auto &&value : values) scratch[count[(static_cast<std::uint64_t>(key(value)) >> shift) & mask]++] = std::move(value);
		values.swap(scratch);
	}
}

template <class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key key) {
	stable_radix_sort<8>(values, std::move(key));
}


#line 5 "geometry/point.hpp"
#include <cmath>
#line 7 "geometry/point.hpp"
#include <istream>
#include <numbers>
#include <optional>
#include <ostream>
#line 12 "geometry/point.hpp"

namespace geometry {
using Real = long double;
inline constexpr Real epsilon = 1e-12L;
inline constexpr Real pi = std::numbers::pi_v<Real>;
enum class Location { outside = -1, boundary = 0, inside = 1 };

template<class T>
inline constexpr bool integer = std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>;
template<class T>
using wide_t = std::conditional_t<integer<T>, __int128_t, Real>;

template<class T>
struct Point {
	static_assert((std::is_arithmetic_v<T> && std::is_signed_v<T>) || std::is_same_v<T, __int128_t>);
	T x{}, y{};
	constexpr Point() = default;
	constexpr Point(T x, T y) : x(x), y(y) {}
	template<class U> explicit constexpr Point(const Point<U>& p) : x(static_cast<T>(p.x)), y(static_cast<T>(p.y)) {}
	constexpr bool operator==(const Point&) const = default;
	constexpr bool operator<(const Point& p) const { return x < p.x || (x == p.x && y < p.y); }
	constexpr Point operator-() const { return {-x, -y}; }
	constexpr Point& operator+=(const Point& p) { x += p.x; y += p.y; return *this; }
	constexpr Point& operator-=(const Point& p) { x -= p.x; y -= p.y; return *this; }
	constexpr Point& operator*=(T c) { x *= c; y *= c; return *this; }
	constexpr Point& operator/=(T c) { x /= c; y /= c; return *this; }
	friend constexpr Point operator+(Point p, const Point& q) { p += q; return p; }
	friend constexpr Point operator-(Point p, const Point& q) { p -= q; return p; }
	friend constexpr Point operator*(Point p, T c) { p *= c; return p; }
	friend constexpr Point operator*(T c, Point p) { p *= c; return p; }
	friend constexpr Point operator/(Point p, T c) { p /= c; return p; }
	friend std::istream& operator>>(std::istream& in, Point& p) { return in >> p.x >> p.y; }
	friend std::ostream& operator<<(std::ostream& out, const Point& p) { return out << p.x << ' ' << p.y; }
};

using IPoint = Point<std::int64_t>;
using FPoint = Point<Real>;

template<class T>
constexpr int sign(T value, Real eps = epsilon) {
	if constexpr (integer<T>) return (value > 0) - (value < 0);
	else return (value > eps) - (value < -eps);
}
template<class T>
constexpr wide_t<T> dot(Point<T> a, Point<T> b) {
	return wide_t<T>(a.x) * b.x + wide_t<T>(a.y) * b.y;
}
template<class T>
constexpr wide_t<T> cross(Point<T> a, Point<T> b) {
	return wide_t<T>(a.x) * b.y - wide_t<T>(a.y) * b.x;
}
template<class T>
constexpr wide_t<T> cross(Point<T> a, Point<T> b, Point<T> c) {
	using W = wide_t<T>;
	return (W(b.x) - a.x) * (W(c.y) - a.y) - (W(b.y) - a.y) * (W(c.x) - a.x);
}
template<class T>
constexpr wide_t<T> norm2(Point<T> p) { return dot(p, p); }
template<class T>
constexpr wide_t<T> distance_squared(Point<T> a, Point<T> b) {
	using W = wide_t<T>;
	W x = W(a.x) - b.x, y = W(a.y) - b.y;
	return x * x + y * y;
}
template<class T>
constexpr int orientation(Point<T> a, Point<T> b, Point<T> c, Real eps = epsilon) { return sign(cross(a, b, c), eps); }
template<class T>
constexpr int ccw(Point<T> a, Point<T> b, Point<T> c, Real eps = epsilon) {
	if (int turn = orientation(a, b, c, eps)) return turn;
	using W = wide_t<T>;
	W x = W(b.x) - a.x, y = W(b.y) - a.y, u = W(c.x) - a.x, v = W(c.y) - a.y;
	if (sign(x * u + y * v, eps) < 0) return 2;
	if (sign(u * u + v * v - x * x - y * y, eps) > 0) return -2;
	return 0;
}
template<class T>
constexpr FPoint to_real(Point<T> p) { return FPoint(p); }
template<class T>
Real length(Point<T> p) { return std::hypot(Real(p.x), Real(p.y)); }
template<class T>
Real distance(Point<T> a, Point<T> b) {
	return std::hypot(Real(wide_t<T>(a.x) - b.x), Real(wide_t<T>(a.y) - b.y));
}
template<class T>
constexpr bool almost_equal(Point<T> a, Point<T> b, Real eps = epsilon) {
	if constexpr (integer<T>) return a == b;
	else return std::abs(Real(a.x) - b.x) <= eps && std::abs(Real(a.y) - b.y) <= eps;
}
template<class T>
constexpr Point<T> rotate90(Point<T> p) { return {-p.y, p.x}; }
template<class T>
Real argument(Point<T> p) { return std::atan2(Real(p.y), Real(p.x)); }
template<class T>
Real angle(Point<T> a, Point<T> b) { return std::atan2(Real(cross(a, b)), Real(dot(a, b))); }
template<class T>
FPoint rotate(Point<T> p, Real theta) {
	Real c = std::cos(theta), s = std::sin(theta);
	return {Real(p.x) * c - Real(p.y) * s, Real(p.x) * s + Real(p.y) * c};
}
template<class T>
std::optional<FPoint> unit(Point<T> p) {
	Real n = length(p);
	if (!n) return std::nullopt;
	return to_real(p) / n;
}
template<class T>
struct PolarLess {
	bool operator()(Point<T> a, Point<T> b) const {
		if (a == Point<T>{} || b == Point<T>{}) return a == Point<T>{} && b != Point<T>{};
		auto half = [](Point<T> p) { return p.y < 0 || (p.y == 0 && p.x < 0); };
		if (half(a) != half(b)) return half(a) < half(b);
		if constexpr (integer<T> && sizeof(T) <= 4) {
			auto c = std::int64_t(a.x) * b.y - std::int64_t(a.y) * b.x;
			if (c) return c > 0;
			auto norm = [](Point<T> p) { return std::uint64_t(std::int64_t(p.x) * p.x) + std::uint64_t(std::int64_t(p.y) * p.y); };
			return norm(a) < norm(b);
		} else {
			if (auto c = cross(a, b); c != 0) return c > 0;
			return norm2(a) < norm2(b);
		}
	}
};

template<class T>
void polar_sort(std::vector<Point<T>> &points) {
	if constexpr (integer<T> && sizeof(T) <= 4) {
		if (points.size() >= 256) {
			struct Entry { std::uint64_t key; Point<T> point; };
			std::vector<Entry> order;
			order.reserve(points.size());
			constexpr std::uint64_t scale = (std::uint64_t(1) << 61) - 1;
			for (auto p : points) {
				std::int64_t x = p.x, y = p.y;
				std::uint64_t a = x < 0 ? -x : x, b = y < 0 ? -y : y;
				unsigned quadrant = y < 0 ? (x < 0 ? 2 : 3) : (x < 0 ? 1 : 0);
				unsigned octant = quadrant * 2 + ((a < b) ^ (quadrant & 1));
				std::uint64_t ratio = a || b ? std::min(scale, std::uint64_t(double(std::min(a, b)) / double(std::max(a, b)) * double(scale))) : 0;
				if (octant & 1) ratio = scale - ratio;
				order.push_back({(std::uint64_t(octant) << 61) | ratio, p});
			}
			stable_radix_sort<11>(order, [](const Entry &entry) { return entry.key; });
			for (std::size_t first = 0; first < order.size();) {
				std::size_t last = first + 1;
				while (last < order.size() && order[last].key == order[first].key) ++last;
				auto less = [](const Entry &a, const Entry &b) { return PolarLess<T>{}(a.point, b.point); };
				if (last - first <= 8 || !std::is_sorted(order.begin() + first, order.begin() + last, less))
					std::sort(order.begin() + first, order.begin() + last, less);
				first = last;
			}
			for (std::size_t i = 0; i < points.size(); ++i) points[i] = order[i].point;
			return;
		}
	}
	std::sort(points.begin(), points.end(), PolarLess<T>{});
}

}


#line 8 "geometry/linear.hpp"

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


#line 5 "geometry/polygon.hpp"
#include <cassert>
#line 9 "geometry/polygon.hpp"

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


#line 1 "components/closest_pair.hpp"



#line 1 "components/splitmix64.hpp"



#line 5 "components/splitmix64.hpp"

inline std::uint64_t splitmix64(std::uint64_t x) {
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
	return x ^ (x >> 31);
}
class SplitMix64 {
	std::uint64_t state;

  public:
	explicit SplitMix64(std::uint64_t seed = 0) : state(seed) {}
	std::uint64_t operator()() { return splitmix64(state += 0x9e3779b97f4a7c15ULL); }
};


#line 7 "components/closest_pair.hpp"
#include <bit>
#line 14 "components/closest_pair.hpp"

namespace closest_pair_detail {
inline std::optional<std::pair<std::uint64_t, std::uint64_t>> packed(std::vector<std::uint64_t> points, std::uint32_t max_x, std::uint64_t budget) {
	using U = std::uint64_t;
	assert(points.size() >= 2 && points.size() <= INT32_MAX);
	std::uint32_t n = points.size();
	auto distance = [](U a, U b) {
		std::int64_t x = std::int64_t(std::uint32_t(a)) - std::uint32_t(b), y = std::int64_t(a >> 32) - std::int64_t(b >> 32);
		return U(x * x) + U(y * y);
	};
	U a = points[0], b = points[1], best = distance(a, b);
	if (!best) return std::pair{a, b};
	SplitMix64 random(n);
	for (std::uint32_t i = 0; i < (n + 3) / 4; ++i) {
		auto x = std::uint32_t((__uint128_t(random()) * n) >> 64), y = std::uint32_t((__uint128_t(random()) * n) >> 64);
		if (x == y) continue;
		U d = distance(points[x], points[y]);
		if (d < best) {
			a = points[x], b = points[y], best = d;
			if (!d) return std::pair{a, b};
		}
	}
	auto limit_for = [](U d) {
		U r = std::sqrt(double(d - 1));
		while ((r + 1) * (r + 1) < d) ++r;
		while (r * r >= d) --r;
		return r;
	};
	U limit = limit_for(best);
	unsigned shift = std::bit_width(limit), x_bits = std::bit_width(max_x);
	auto key = [=](U p) { return ((p >> 32 >> shift) << x_bits) | std::uint32_t(p); };
	U maximum = 0;
	for (U p : points) maximum = std::max(maximum, key(p));
	unsigned bits = std::min(11U, unsigned(std::bit_width(n))), buckets = 1U << bits, mask = buckets - 1;
	std::vector<U> temporary(n);
	for (unsigned digit = 0; digit < 64 && (maximum >> digit); digit += bits) {
		std::array<std::uint32_t, 2048> count{};
		for (U p : points) ++count[(key(p) >> digit) & mask];
		std::uint32_t sum = 0;
		for (unsigned i = 0; i < buckets; ++i) { auto old = count[i]; count[i] = sum; sum += old; }
		for (U p : points) temporary[count[(key(p) >> digit) & mask]++] = p;
		points.swap(temporary);
	}
	auto attempt = [&](U x, U y) {
		U d = distance(x, y);
		if (d < best) {
			a = x, b = y, best = d;
			if (d) limit = limit_for(d);
		}
	};
	std::uint32_t prev_begin = 0, prev_end = 0;
	U prev_lane = UINT64_MAX;
	for (std::uint32_t first = 0; first < n;) {
		U lane = points[first] >> 32 >> shift;
		std::uint32_t end = first + 1;
		while (end < n && (points[end] >> 32 >> shift) == lane) ++end;
		std::uint32_t left = prev_begin;
		bool adjacent = prev_begin < prev_end && prev_lane + 1 == lane;
		for (std::uint32_t i = first; i < end; ++i) {
			U p = points[i], x = std::uint32_t(p);
			for (std::uint32_t j = i; j > first;) {
				--j;
				if (x - std::uint32_t(points[j]) > limit) break;
				if (!budget--) return std::nullopt;
				attempt(p, points[j]);
				if (!best) return std::pair{a, b};
			}
			if (!adjacent || (p >> 32) - (lane << shift) + 1 > limit) continue;
			while (left < prev_end && U(std::uint32_t(points[left])) + limit < x) ++left;
			for (std::uint32_t j = left; j < prev_end && U(std::uint32_t(points[j])) <= x + limit; ++j) {
				if (!budget--) return std::nullopt;
				attempt(p, points[j]);
				if (!best) return std::pair{a, b};
			}
		}
		prev_begin = first, prev_end = end, prev_lane = lane, first = end;
	}
	return std::pair{a, b};
}
}


#line 1 "components/packed_convex_hull.hpp"



#line 9 "components/packed_convex_hull.hpp"

namespace packed_convex_hull_detail {
struct Entry {
	std::uint64_t key;
	int index;
};
inline std::int64_t cross(std::uint64_t a, std::uint64_t b, std::uint64_t c) {
	std::int64_t x = a >> 32, y = std::uint32_t(a);
	return (std::int64_t(b >> 32) - x) * (std::int64_t(std::uint32_t(c)) - y)
		- (std::int64_t(std::uint32_t(b)) - y) * (std::int64_t(c >> 32) - x);
}
inline std::uint64_t key(std::uint64_t value) { return value; }
inline std::uint64_t key(Entry value) { return value.key; }
template <class Value>
std::vector<Value> build(std::vector<Value> order, bool keep_collinear) {
	if (order.size() < 256) std::sort(order.begin(), order.end(), [](const Value &a, const Value &b) { return key(a) < key(b); });
	else stable_radix_sort<11>(order, [](const Value &value) { return key(value); });
	order.erase(std::unique(order.begin(), order.end(), [](const Value &a, const Value &b) { return key(a) == key(b); }), order.end());
	if (order.size() <= 2 || (keep_collinear && std::all_of(order.begin() + 2, order.end(), [&](const Value &value) {
		return !cross(key(order[0]), key(order[1]), key(value));
	}))) {
		return order;
	}
	std::vector<Value> hull;
	hull.reserve(order.size() * 2);
	auto append = [&](Value value, std::size_t base) {
		while (hull.size() >= base) {
			auto turn = cross(key(hull[hull.size() - 2]), key(hull.back()), key(value));
			if (turn > 0 || (keep_collinear && !turn)) break;
			hull.pop_back();
		}
		hull.push_back(value);
	};
	for (auto value : order) append(value, 2);
	std::size_t base = hull.size() + 1;
	for (std::size_t i = order.size() - 1; i-- > 0;) append(order[i], base);
	hull.pop_back();
	return hull;
}
inline std::pair<std::uint64_t, std::uint64_t> diameter(const std::vector<std::uint64_t> &hull) {
	std::size_t n = hull.size();
	std::uint64_t a = hull[0], b = a, best = 0;
	auto distance = [](std::uint64_t p, std::uint64_t q) {
		std::int64_t x = std::int64_t(p >> 32) - std::int64_t(q >> 32);
		std::int64_t y = std::int64_t(std::uint32_t(p)) - std::uint32_t(q);
		return std::uint64_t(x * x + y * y);
	};
	for (std::size_t i = 0, j = n > 1; i < n; ++i) {
		std::size_t next = i + 1 == n ? 0 : i + 1;
		while (true) {
			std::size_t after = j + 1 == n ? 0 : j + 1;
			if (cross(hull[i], hull[next], hull[after]) <= cross(hull[i], hull[next], hull[j])) break;
			j = after;
		}
		for (auto p : {hull[i], hull[next]}) {
			auto d = distance(p, hull[j]);
			if (d > best) a = p, b = hull[j], best = d;
		}
	}
	return {a, b};
}

}


#line 9 "geometry/convex.hpp"
#include <limits>
#include <numeric>
#line 13 "geometry/convex.hpp"

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
		if (points.size() >= 4) {
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
		if (points.size() >= 4) {
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
	assert(points.size() <= std::size_t(std::numeric_limits<int>::max()));
	if (points.size() < 2) return std::nullopt;
	if constexpr (integer<T> && sizeof(T) <= 8) {
		if (points.size() >= 64) {
			T min_x = points[0].x, max_x = min_x, min_y = points[0].y, max_y = min_y;
			for (auto p : points) {
				min_x = std::min(min_x, p.x), max_x = std::max(max_x, p.x);
				min_y = std::min(min_y, p.y), max_y = std::max(max_y, p.y);
			}
			if (wide_t<T>(max_x) - min_x <= INT32_MAX && wide_t<T>(max_y) - min_y <= INT32_MAX) {
				auto pack = [&](Point<T> p) {
					return (std::uint64_t(wide_t<T>(p.x) - min_x) << 32) | std::uint32_t(wide_t<T>(p.y) - min_y);
				};
				std::vector<std::uint64_t> order(points.size());
				for (std::size_t i = 0; i < points.size(); ++i) order[i] = pack(points[i]);
				auto hull = packed_convex_hull_detail::build(std::move(order), false);
				if (hull.size() == 1) return PointPair<T>{0, 1, 0};
				auto [a, b] = packed_convex_hull_detail::diameter(hull);
				int first = -1, second = -1;
				for (int i = 0; i < int(points.size()) && (first < 0 || second < 0); ++i) {
					auto value = pack(points[i]);
					if (value == a) first = i;
					if (value == b) second = i;
				}
				return PointPair<T>{first, second, distance_squared(points[first], points[second])};
			}
		}
	}
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


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#line 18 "components/fast_io.hpp"
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 4 "test/geometry_furthest_pair.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0; input.read(t);
	while (t--) {
		int n = 0; input.read(n);
		std::vector<geometry::IPoint> points(n);
		for (auto &p : points) input.read(p.x), input.read(p.y);
		auto pair = geometry::furthest_pair(points);
		output.write(pair->first), output.write(' '), output.println(pair->second);
	}
}
