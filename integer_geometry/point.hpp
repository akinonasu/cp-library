#ifndef INTEGER_GEOMETRY_POINT_HPP
#define INTEGER_GEOMETRY_POINT_HPP

#include "../geometry/point.hpp"
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <istream>
#include <ostream>
#include <type_traits>
#include <vector>

namespace integer_geometry {
using Integer = std::int64_t;
using Wide = __int128_t;

template<class T>
concept Coordinate = std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8;

template<Coordinate T>
using wide_t = Wide;

enum class Location { outside = -1, boundary = 0, inside = 1 };

template<Coordinate T>
struct BasicPoint {
	T x{}, y{};
	constexpr BasicPoint() = default;
	constexpr BasicPoint(T x, T y) : x(x), y(y) {}
	template<class U, class V> requires (std::is_floating_point_v<U> || std::is_floating_point_v<V>)
	BasicPoint(U, V) = delete;
	template<Coordinate U>
	explicit constexpr BasicPoint(const BasicPoint<U> &p) : x(static_cast<T>(p.x)), y(static_cast<T>(p.y)) {}
	constexpr bool operator==(const BasicPoint &) const = default;
	constexpr bool operator<(const BasicPoint &p) const { return x < p.x || (x == p.x && y < p.y); }
	constexpr BasicPoint operator-() const { return {T(-x), T(-y)}; }
	constexpr BasicPoint &operator+=(const BasicPoint &p) { x += p.x; y += p.y; return *this; }
	constexpr BasicPoint &operator-=(const BasicPoint &p) { x -= p.x; y -= p.y; return *this; }
	constexpr BasicPoint &operator*=(T c) { x *= c; y *= c; return *this; }
	template<std::floating_point U> BasicPoint &operator*=(U) = delete;
	friend constexpr BasicPoint operator+(BasicPoint p, const BasicPoint &q) { p += q; return p; }
	friend constexpr BasicPoint operator-(BasicPoint p, const BasicPoint &q) { p -= q; return p; }
	friend constexpr BasicPoint operator*(BasicPoint p, T c) { p *= c; return p; }
	friend constexpr BasicPoint operator*(T c, BasicPoint p) { p *= c; return p; }
	friend std::istream &operator>>(std::istream &in, BasicPoint &p) { return in >> p.x >> p.y; }
	friend std::ostream &operator<<(std::ostream &out, const BasicPoint &p) { return out << p.x << ' ' << p.y; }
};

using Point = BasicPoint<Integer>;

template<Coordinate T, std::floating_point U>
BasicPoint<T> operator*(BasicPoint<T>, U) = delete;
template<Coordinate T, std::floating_point U>
BasicPoint<T> operator*(U, BasicPoint<T>) = delete;
template<class T> requires (std::is_integral_v<T> || std::same_as<T, Wide>)
constexpr int sign(T value) { return (value > 0) - (value < 0); }

template<Coordinate T>
constexpr Wide dot(const BasicPoint<T> &a, const BasicPoint<T> &b) { return Wide(a.x) * b.x + Wide(a.y) * b.y; }
template<Coordinate T>
constexpr Wide cross(const BasicPoint<T> &a, const BasicPoint<T> &b) { return Wide(a.x) * b.y - Wide(a.y) * b.x; }
template<Coordinate T>
constexpr Wide cross(const BasicPoint<T> &a, const BasicPoint<T> &b, const BasicPoint<T> &c) {
	return (Wide(b.x) - a.x) * (Wide(c.y) - a.y) - (Wide(b.y) - a.y) * (Wide(c.x) - a.x);
}
template<Coordinate T>
constexpr Wide norm2(const BasicPoint<T> &p) { return dot(p, p); }
template<Coordinate T>
constexpr Wide distance_squared(const BasicPoint<T> &a, const BasicPoint<T> &b) {
	Wide x = Wide(a.x) - b.x, y = Wide(a.y) - b.y;
	return x * x + y * y;
}
template<Coordinate T>
constexpr int orientation(const BasicPoint<T> &a, const BasicPoint<T> &b, const BasicPoint<T> &c) { return sign(cross(a, b, c)); }
template<Coordinate T>
constexpr int ccw(const BasicPoint<T> &a, const BasicPoint<T> &b, const BasicPoint<T> &c) {
	if (int turn = orientation(a, b, c)) return turn;
	Wide x = Wide(b.x) - a.x, y = Wide(b.y) - a.y, u = Wide(c.x) - a.x, v = Wide(c.y) - a.y;
	if (x * u + y * v < 0) return 2;
	return u * u + v * v > x * x + y * y ? -2 : 0;
}
template<Coordinate T>
constexpr BasicPoint<T> rotate90(const BasicPoint<T> &p) { return {T(-p.y), p.x}; }

template<Coordinate T>
struct PolarLess {
	BasicPoint<T> center{};
	bool operator()(const BasicPoint<T> &a, const BasicPoint<T> &b) const {
		if (center == BasicPoint<T>{}) return geometry::PolarLess<T>{}({a.x, a.y}, {b.x, b.y});
		Wide x = Wide(a.x) - center.x, y = Wide(a.y) - center.y;
		Wide u = Wide(b.x) - center.x, v = Wide(b.y) - center.y;
		bool zero_a = x == 0 && y == 0, zero_b = u == 0 && v == 0;
		if (zero_a || zero_b) return zero_a && !zero_b;
		bool half_a = y < 0 || (y == 0 && x < 0), half_b = v < 0 || (v == 0 && u < 0);
		if (half_a != half_b) return half_a < half_b;
		if (Wide turn = x * v - y * u; turn != 0) return turn > 0;
		return x * x + y * y < u * u + v * v;
	}
};

template<Coordinate T>
void polar_sort(std::vector<BasicPoint<T>> &points, BasicPoint<T> center = {}) {
	if (center == BasicPoint<T>{}) {
		if (points.size() >= 256 && std::all_of(points.begin(), points.end(), [](BasicPoint<T> point) {
			return INT32_MIN <= point.x && point.x <= INT32_MAX && INT32_MIN <= point.y && point.y <= INT32_MAX;
		})) {
			std::vector<geometry::Point<std::int32_t>> converted;
			converted.reserve(points.size());
			for (auto point : points) converted.push_back({std::int32_t(point.x), std::int32_t(point.y)});
			geometry::polar_sort(converted);
			for (std::size_t i = 0; i < points.size(); ++i) points[i] = {T(converted[i].x), T(converted[i].y)};
			return;
		}
		std::sort(points.begin(), points.end(), [](BasicPoint<T> a, BasicPoint<T> b) {
			return geometry::PolarLess<T>{}({a.x, a.y}, {b.x, b.y});
		});
	} else std::sort(points.begin(), points.end(), PolarLess<T>{center});
}
}

#endif
