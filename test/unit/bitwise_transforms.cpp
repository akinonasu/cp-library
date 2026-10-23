#include "components/bitwise_zeta.hpp"
#include "components/walsh_hadamard.hpp"
#include "convolution/bitwise_convolution.hpp"
#include "math/mint.hpp"
#include "test/fixtures/bitwise_transforms_reference.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

struct Noncopyable {
	std::uint64_t value = 0;
	Noncopyable() = default;
	Noncopyable(const Noncopyable &) = delete;
	Noncopyable &operator=(const Noncopyable &) = delete;
	Noncopyable &operator+=(const Noncopyable &other) { value = value * 31337 + other.value; return *this; }
	Noncopyable &operator-=(const Noncopyable &other) { value = value * 1000003 - other.value; return *this; }
};

struct Ordered {
	std::uint64_t value;
	Ordered() = delete;
	explicit Ordered(std::uint64_t x) : value(x) {}
	Ordered &operator+=(const Ordered &other) { value = value * 31337 + other.value; return *this; }
	Ordered &operator-=(const Ordered &other) { value = value * 1000003 - other.value; return *this; }
	Ordered operator+(const Ordered &other) const { return Ordered(value * 31337 + other.value); }
	Ordered operator-(const Ordered &other) const { return Ordered(value * 1000003 - other.value); }
	bool operator==(const Ordered &) const = default;
};

struct Expression { std::uint64_t value; };
struct AssignmentOnly {
	std::uint64_t value;
	AssignmentOnly() = delete;
	explicit AssignmentOnly(std::uint64_t x) : value(x) {}
	AssignmentOnly(const AssignmentOnly &) = default;
	AssignmentOnly &operator=(const AssignmentOnly &) = delete;
	AssignmentOnly &operator=(Expression x) { value = x.value; return *this; }
	Expression operator+(const AssignmentOnly &other) const { return {value + other.value}; }
	Expression operator-(const AssignmentOnly &other) const { return {value - other.value}; }
	bool operator==(const AssignmentOnly &) const = default;
};

static_assert(!std::is_constructible_v<AssignmentOnly, Expression>);

template <class T>
bool identical(const T &a, const T &b) {
	if constexpr (std::is_same_v<T, double>) return std::bit_cast<std::uint64_t>(a) == std::bit_cast<std::uint64_t>(b);
	else if constexpr (std::is_same_v<T, float>) return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
	else if constexpr (std::is_same_v<T, Noncopyable>) return a.value == b.value;
	else return a == b;
}

template <class T>
void equal(const std::vector<T> &a, const std::vector<T> &b) {
	assert(a.size() == b.size());
	for (std::size_t i = 0; i < a.size(); ++i) assert(identical(a[i], b[i]));
}

template <bool inverse, bool superset, class T>
void check_zeta(const std::vector<T> &input) {
	auto a = input, b = input;
	bitwise_transforms_baseline::bitwise_zeta<inverse, superset>(a);
	bitwise_zeta<inverse, superset>(b);
	equal(a, b);
}

template <class T>
void check_walsh(const std::vector<T> &input) {
	auto a = input, b = input;
	bitwise_transforms_baseline::walsh_hadamard(a);
	walsh_hadamard(b);
	equal(a, b);
}

template <class T>
void check(const std::vector<T> &input) {
	check_zeta<false, false>(input);
	check_zeta<false, true>(input);
	check_zeta<true, false>(input);
	check_zeta<true, true>(input);
	check_walsh(input);
}

template <bool inverse, bool superset>
void check_noncopyable(std::size_t n) {
	std::vector<Noncopyable> a(n), b(n);
	for (std::size_t i = 0; i < n; ++i) a[i].value = b[i].value = i * 719 + 1;
	bitwise_transforms_baseline::bitwise_zeta<inverse, superset>(a);
	bitwise_zeta<inverse, superset>(b);
	equal(a, b);
}

void check_convolution() {
	std::mt19937 rng(9871);
	for (int n : {1, 2, 3, 7, 32, 63, 1025}) {
		std::vector<mint> a(n), b(n), expected_and(std::bit_ceil(unsigned(n))), expected_or(expected_and), expected_xor(expected_and);
		for (auto &x : a) x = rng();
		for (auto &x : b) x = rng();
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) {
				expected_and[i & j] += a[i] * b[j];
				expected_or[i | j] += a[i] * b[j];
				expected_xor[i ^ j] += a[i] * b[j];
			}
		assert(and_convolution(a, b) == expected_and);
		assert(or_convolution(a, b) == expected_or);
		assert(xor_convolution(a, b) == expected_xor);
	}
}

int main() {
	std::mt19937_64 rng(20261017);
	for (int exponent = -1; exponent <= 16; ++exponent) {
		std::size_t n = exponent < 0 ? 0 : std::size_t(1) << exponent;
		std::vector<std::uint64_t> unsigned_values(n);
		std::vector<std::int64_t> signed_values(n);
		std::vector<std::uint8_t> narrow_unsigned(n);
		std::vector<std::int8_t> narrow_signed(n);
		std::vector<mint> modular(n);
		std::vector<double> floating(n);
		std::vector<float> single(n);
		std::vector<Ordered> ordered;
		std::vector<AssignmentOnly> expression;
		std::vector<bool> bits(n);
		ordered.reserve(n);
		expression.reserve(n);
		for (std::size_t i = 0; i < n; ++i) {
			unsigned_values[i] = rng();
			signed_values[i] = std::int64_t(rng() % 2001) - 1000;
			narrow_unsigned[i] = rng();
			narrow_signed[i] = rng();
			modular[i] = rng();
			floating[i] = std::ldexp(double(std::int64_t(rng() % 2000001) - 1000000), int(rng() % 1601) - 800);
			single[i] = std::ldexp(float(std::int64_t(rng() % 2001) - 1000), int(rng() % 161) - 80);
			ordered.emplace_back(rng());
			expression.emplace_back(rng());
			bits[i] = rng() & 1;
		}
		check(unsigned_values);
		check(signed_values);
		check(narrow_unsigned);
		check(narrow_signed);
		check(modular);
		check(floating);
		check(single);
		check(ordered);
		check_walsh(expression);
		check_walsh(bits);
		check_noncopyable<false, false>(n);
		check_noncopyable<false, true>(n);
		check_noncopyable<true, false>(n);
		check_noncopyable<true, true>(n);
	}
	for (double x : {0.0, -0.0, std::numeric_limits<double>::denorm_min(), -std::numeric_limits<double>::denorm_min()})
		check(std::vector<double>(2048, x));
	check(std::vector<double>{1e30, -1e30, 1.0, -1.0});
	check(std::vector<float>{1e30f, -1e30f, 1.0f, -1.0f});
	check_convolution();
}
