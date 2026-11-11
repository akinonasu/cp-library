#ifndef INTEGER_GEOMETRY_POINT_HPP
#define INTEGER_GEOMETRY_POINT_HPP

#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <cassert>
#include <concepts>
#include <istream>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

namespace integer_geometry {
using Integer = boost::multiprecision::cpp_int;
using Rational = boost::multiprecision::cpp_rational;

template<class T>
concept Coordinate = std::same_as<T, Integer> || std::same_as<T, Rational>;

enum class Location { outside = -1, boundary = 0, inside = 1 };

template<Coordinate T>
struct BasicPoint {
	T x{}, y{};
	BasicPoint() = default;
	BasicPoint(T x, T y) : x(std::move(x)), y(std::move(y)) {}
	template<class U, class V> requires (std::is_floating_point_v<U> || std::is_floating_point_v<V>)
	BasicPoint(U, V) = delete;
	template<Coordinate U> requires std::same_as<T, Rational>
	explicit BasicPoint(const BasicPoint<U> &p) : x(p.x), y(p.y) {}
	bool operator==(const BasicPoint &) const = default;
	bool operator<(const BasicPoint &p) const { return x < p.x || (x == p.x && y < p.y); }
	BasicPoint operator-() const { return {-x, -y}; }
	BasicPoint &operator+=(const BasicPoint &p) { x += p.x; y += p.y; return *this; }
	BasicPoint &operator-=(const BasicPoint &p) { x -= p.x; y -= p.y; return *this; }
	BasicPoint &operator*=(T c) { x *= c; y *= c; return *this; }
	template<std::floating_point U> BasicPoint &operator*=(U) = delete;
	BasicPoint &operator/=(T c) requires std::same_as<T, Rational> {
		assert(c != 0);
		x /= c; y /= c;
		return *this;
	}
	template<std::floating_point U> BasicPoint &operator/=(U) = delete;
	friend BasicPoint operator+(BasicPoint p, const BasicPoint &q) { p += q; return p; }
	friend BasicPoint operator-(BasicPoint p, const BasicPoint &q) { p -= q; return p; }
	friend BasicPoint operator*(BasicPoint p, const T &c) { p *= c; return p; }
	friend BasicPoint operator*(const T &c, BasicPoint p) { p *= c; return p; }
	friend std::istream &operator>>(std::istream &in, BasicPoint &p) { return in >> p.x >> p.y; }
	friend std::ostream &operator<<(std::ostream &out, const BasicPoint &p) { return out << p.x << ' ' << p.y; }
};

using Point = BasicPoint<Integer>;
using RPoint = BasicPoint<Rational>;

template<Coordinate T>
RPoint to_rational(const BasicPoint<T> &p) { return {Rational(p.x), Rational(p.y)}; }

template<Coordinate T>
RPoint operator/(const BasicPoint<T> &p, const Rational &c) {
	assert(c != 0);
	return {Rational(p.x) / c, Rational(p.y) / c};
}

template<Coordinate T, std::floating_point U>
BasicPoint<T> operator*(BasicPoint<T>, U) = delete;
template<Coordinate T, std::floating_point U>
BasicPoint<T> operator*(U, BasicPoint<T>) = delete;
template<Coordinate T, std::floating_point U>
RPoint operator/(const BasicPoint<T> &, U) = delete;

template<class T> requires (!std::is_floating_point_v<T>)
int sign(const T &value) { return (value > 0) - (value < 0); }

template<Coordinate T>
T dot(const BasicPoint<T> &a, const BasicPoint<T> &b) { return a.x * b.x + a.y * b.y; }
template<Coordinate T>
T cross(const BasicPoint<T> &a, const BasicPoint<T> &b) { return a.x * b.y - a.y * b.x; }
template<Coordinate T>
T cross(const BasicPoint<T> &a, const BasicPoint<T> &b, const BasicPoint<T> &c) {
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
template<Coordinate T>
T norm2(const BasicPoint<T> &p) { return dot(p, p); }
template<Coordinate T>
T distance_squared(const BasicPoint<T> &a, const BasicPoint<T> &b) { return norm2(a - b); }
template<Coordinate T>
int orientation(const BasicPoint<T> &a, const BasicPoint<T> &b, const BasicPoint<T> &c) { return sign(cross(a, b, c)); }
template<Coordinate T>
int ccw(const BasicPoint<T> &a, const BasicPoint<T> &b, const BasicPoint<T> &c) {
	if (int turn = orientation(a, b, c)) return turn;
	auto u = b - a, v = c - a;
	if (dot(u, v) < 0) return 2;
	return norm2(v) > norm2(u) ? -2 : 0;
}
template<Coordinate T>
BasicPoint<T> rotate90(const BasicPoint<T> &p) { return {-p.y, p.x}; }

template<Coordinate T>
struct PolarLess {
	BasicPoint<T> center{};
	bool operator()(const BasicPoint<T> &a, const BasicPoint<T> &b) const {
		auto p = a - center, q = b - center;
		if (p == BasicPoint<T>{} || q == BasicPoint<T>{}) return p == BasicPoint<T>{} && q != BasicPoint<T>{};
		auto half = [](const BasicPoint<T> &v) { return v.y < 0 || (v.y == 0 && v.x < 0); };
		if (half(p) != half(q)) return half(p) < half(q);
		if (auto turn = cross(p, q); turn != 0) return turn > 0;
		return norm2(p) < norm2(q);
	}
};

template<Coordinate T>
void polar_sort(std::vector<BasicPoint<T>> &points, BasicPoint<T> center = {}) {
	std::sort(points.begin(), points.end(), PolarLess<T>{std::move(center)});
}
}

#endif
