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

template <int kind>
std::vector<mint> reference_convolution(std::vector<mint> a, std::vector<mint> b) {
	std::size_t n = std::bit_ceil(std::max(a.size(), b.size()));
	a.resize(n), b.resize(n);
	if constexpr (kind == 2) {
		bitwise_transforms_baseline::walsh_hadamard(a);
		bitwise_transforms_baseline::walsh_hadamard(b);
	} else {
		bitwise_transforms_baseline::bitwise_zeta<false, kind == 0>(a);
		bitwise_transforms_baseline::bitwise_zeta<false, kind == 0>(b);
	}
	for (std::size_t i = 0; i < n; ++i) a[i] *= b[i];
	if constexpr (kind == 2) {
		bitwise_transforms_baseline::walsh_hadamard(a);
		for (auto &x : a) x /= mint(n);
	} else bitwise_transforms_baseline::bitwise_zeta<true, kind == 0>(a);
	return a;
}

void modular_convolution() {
	constexpr unsigned p = mint::mod();
	const std::array<unsigned, 8> values{0, 1, p - 1, p, p + 1, 2 * p - 2, 2 * p - 1, p / 2};
	for (int n : {8, 16, 32, 64, 256, 1024, 2048, 4096}) {
		std::vector<mint> a(n), b(n - 3);
		for (int i = 0; i < n; ++i) a[i] = mint::word(values[i % values.size()]);
		for (int i = 0; i < int(b.size()); ++i) b[i] = mint::word(values[(i * 5 + 3) % values.size()]);
		assert(and_convolution(a, b) == reference_convolution<0>(a, b));
		assert(or_convolution(a, b) == reference_convolution<1>(a, b));
		assert(xor_convolution(a, b) == reference_convolution<2>(a, b));
	}
}

template <int kind, bool inverse>
void unaligned_modular_transform() {
#if defined(__x86_64__) || defined(__i386__)
	using Kernel = bitwise_convolution_detail::Modular;
	if (!Kernel::vectorized()) return;
	alignas(32) std::array<mint, 1032> storage;
	storage.fill(mint::word(127));
	std::vector<mint> expected(1024);
	for (int i = 0; i < 1024; ++i) {
		expected[i] = mint::word(unsigned((std::uint64_t(i) * 19491001) % (2 * mint::mod())));
		storage[i + 1] = expected[i];
	}
	if constexpr (kind == 2) bitwise_transforms_baseline::walsh_hadamard(expected);
	else bitwise_transforms_baseline::bitwise_zeta<inverse, kind == 0>(expected);
	Kernel::transform<kind, inverse>(storage.data() + 1, 1024);
	assert(storage[0].v == 127);
	for (int i = 0; i < 1024; ++i) assert(storage[i + 1] == expected[i]);
	for (int i = 1025; i < int(storage.size()); ++i) assert(storage[i].v == 127);
#endif
}

int main() {
	modular_convolution();
	unaligned_modular_transform<0, false>();
	unaligned_modular_transform<0, true>();
	unaligned_modular_transform<1, false>();
	unaligned_modular_transform<1, true>();
	unaligned_modular_transform<2, false>();
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
