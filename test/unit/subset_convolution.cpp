#include "convolution/subset_convolution.hpp"
#include "convolution/boolean_convolution.hpp"
#include "math/mint.hpp"
#include "test/fixtures/subset_convolution_reference.hpp"
#include "test/fixtures/recurrence_types.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <type_traits>

template <class T>
std::vector<T> reference(const std::vector<T> &a, const std::vector<T> &b) {
	if (a.empty() || b.empty()) return {};
	std::vector<T> c(std::bit_ceil(std::max(a.size(), b.size())), T(0));
	for (std::size_t mask = 0; mask < c.size(); ++mask)
		for (std::size_t sub = mask;; sub = (sub - 1) & mask) {
			if (sub < a.size() && (mask ^ sub) < b.size()) c[mask] += a[sub] * b[mask ^ sub];
			if (!sub) break;
		}
	return c;
}

struct Matrix {
	std::array<std::uint32_t, 4> v;
	explicit Matrix(int n) : v{std::uint32_t(n), 0, 0, std::uint32_t(n)} {}
	Matrix &operator+=(const Matrix &b) {
		for (int i = 0; i < 4; ++i) v[i] += b.v[i];
		return *this;
	}
	Matrix &operator-=(const Matrix &b) {
		for (int i = 0; i < 4; ++i) v[i] -= b.v[i];
		return *this;
	}
	friend Matrix operator*(const Matrix &a, const Matrix &b) {
		Matrix c(0);
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k) c.v[2 * i + j] += a.v[2 * i + k] * b.v[2 * k + j];
		return c;
	}
	friend bool operator==(const Matrix &, const Matrix &) = default;
};

std::mt19937_64 rng(20261017);
template <class T>
T random_value() {
	if constexpr (std::is_same_v<T, Matrix>) {
		Matrix x(0);
		for (auto &y : x.v) y = rng();
		return x;
	} else if constexpr (std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>) return T(rng());
	else return T(int(rng() % 15) - 7);
}

template <class T>
void check() {
	for (int trial = 0; trial < 180; ++trial) {
		std::vector<T> a(rng() % 90, T(0)), b(rng() % 90, T(0));
		for (auto &x : a) x = random_value<T>();
		for (auto &x : b) x = random_value<T>();
		auto copy_a = a, copy_b = b;
		assert(subset_convolution(a, b) == reference(a, b));
		assert(a == copy_a && b == copy_b);
	}
	for (int e = 0; e <= 12; ++e)
		for (int delta : {-1, 0, 1}) {
			std::size_t n = (std::size_t(1) << e) + delta;
			std::vector<T> a(n, T(0)), b(n / 2 + 1, T(0));
			for (auto &x : a) x = random_value<T>();
			for (auto &x : b) x = random_value<T>();
			assert(subset_convolution(a, b) == reference(a, b));
			assert(subset_convolution(a, a) == subset_convolution_baseline::subset_convolution(a, a));
		}
}

void check_boolean() {
	for (int trial = 0; trial < 30; ++trial) {
		std::vector<std::uint32_t> a(1 + rng() % 40), b(1 + rng() % 40);
		for (auto &x : a) x = rng();
		for (auto &x : b) x = rng();
		std::vector<std::uint32_t> expected_or(std::bit_ceil(std::max(a.size(), b.size())));
		std::vector<std::uint32_t> expected_and(expected_or.size(), ~std::uint32_t(0));
		for (std::size_t i = 0; i < a.size(); ++i)
			for (std::size_t j = 0; j < b.size(); ++j)
				if (!(i & j)) {
					expected_or[i | j] |= a[i] & b[j];
					expected_and[i | j] &= a[i] | b[j];
				}
		assert(subset_or_convolution(a, b) == expected_or);
		assert(subset_and_convolution(a, b) == expected_and);
	}
	assert(subset_or_convolution(std::vector<bool>{false, true}, std::vector<bool>{true, false}) == std::vector<bool>({false, true}));
}

struct DynamicModint {
	std::uint32_t value = 0;
	static std::uint32_t mod() { return 998244353; }
	DynamicModint(long long x = 0) : value((x % mod() + mod()) % mod()) {}
	static DynamicModint raw(std::uint32_t x) { DynamicModint a; a.value = x; return a; }
	std::uint32_t val() const { return value; }
	DynamicModint &operator+=(DynamicModint x) { value += x.value; if (value >= mod()) value -= mod(); return *this; }
	DynamicModint &operator-=(DynamicModint x) { value = value >= x.value ? value - x.value : value + mod() - x.value; return *this; }
	friend DynamicModint operator*(DynamicModint a, DynamicModint b) { return raw(std::uint64_t(a.value) * b.value % mod()); }
	bool operator==(const DynamicModint &) const = default;
};

template <class T>
void check_modular() {
	for (int n : {127, 128, 255, 256, 257, 511, 1023, 1024, 1025, 1800}) {
		std::vector<T> a(n), b(n - 11);
		for (auto &x : a) x = T::raw(rng() % T::mod());
		for (auto &x : b) x = T::raw(rng() % T::mod());
		assert(subset_convolution(a, b) == reference(a, b));
	}
	std::vector<T> a(1 << 15), b(a.size());
	for (auto &x : a) x = T::raw(rng() % T::mod());
	for (auto &x : b) x = T::raw(rng() % T::mod());
	assert(subset_convolution(a, b) == subset_convolution_baseline::subset_convolution(a, b));
}

template <std::uint32_t mod>
void check_modular_product() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if (!subset_convolution_detail::avx2()) return;
	using Kernel = subset_convolution_detail::Modular<mod>;
	for (int trial = 0; trial < 16; ++trial) {
		std::vector<std::uint32_t> a(4 * Kernel::width), b(a.size());
		for (auto &x : a) x = trial ? rng() % mod : mod - 1;
		for (auto &x : b) x = trial ? rng() % mod : mod - 1;
		auto original = a;
		Kernel::product(a.data(), b.data(), 4);
		for (int pos = 0; pos < 4; ++pos)
			for (int rank = 0; rank < 20; ++rank) {
				std::uint64_t expected = 0;
				for (int i = 0; i < rank; ++i)
					expected = (expected + std::uint64_t(original[pos * Kernel::width + i]) * b[pos * Kernel::width + rank - i - 1]) % mod;
				assert(a[pos * Kernel::width + rank] == expected);
			}
	}
#endif
}

int main() {
	check<mint>();
	check<std::uint32_t>();
	check<std::uint64_t>();
	check<__int128_t>();
	check<double>();
	check<Matrix>();
	check_boolean();
	static_assert(subset_convolution_detail::SmallModint<RecurrenceModint<3>>::value);
	static_assert(!subset_convolution_detail::SmallModint<RecurrenceModint<2>>::value);
	static_assert(!subset_convolution_detail::SmallModint<RecurrenceModint<2147483647>>::value);
	static_assert(!subset_convolution_detail::SmallModint<DynamicModint>::value);
	check_modular<RecurrenceModint<3>>();
	check_modular<RecurrenceModint<9>>();
	check_modular<RecurrenceModint<15>>();
	check_modular<RecurrenceModint<1000000007>>();
	check_modular<RecurrenceModint<1073741823>>();
	check_modular<RecurrenceModint<2>>();
	check_modular<RecurrenceModint<2147483647>>();
	check_modular<DynamicModint>();
	check_modular_product<3>();
	check_modular_product<998244353>();
	check_modular_product<1073741823>();
	std::vector<mint> a(1 << 16, mint(1));
	auto c = subset_convolution(a, a);
	for (std::size_t mask = 0; mask < c.size(); ++mask) assert(c[mask] == mint(2).pow(std::popcount(mask)));
}
