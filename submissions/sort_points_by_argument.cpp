#line 1 "test/geometry_argument_sort.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/sort_points_by_argument"
#line 1 "geometry/point.hpp"



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
}


#line 1 "components/fast_io.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <charconv>
#include <cstddef>
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


#line 5 "test/geometry_argument_sort.test.cpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; input.read(n);
	std::vector<geometry::Point<std::int32_t>> points(n);
	for (auto &p : points) input.read(p.x), input.read(p.y);
	std::sort(points.begin(), points.end(), geometry::PolarLess<std::int32_t>{});
	auto lower = std::find_if(points.begin(), points.end(), [](auto p) { return p.y < 0; });
	std::rotate(points.begin(), lower, points.end());
	for (auto p : points) output.write(p.x), output.write(' '), output.println(p.y);
}
