#ifndef GEOMETRY_POINT_HPP
#define GEOMETRY_POINT_HPP

#include "../components/radix_sort.hpp"
#include <cmath>
#include <cstdint>
#include <istream>
#include <numbers>
#include <optional>
#include <ostream>
#include <type_traits>

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

#endif
