#include "fps/linear_recurrence_generic.hpp"
#if defined(CP_FPS_API) || defined(CP_FPS_RECURRENCE_API) || defined(CP_CONVOLUTION_MOD_BACKEND)
#error "Generic recurrence must not select an arithmetic backend"
#endif
#include "fps/linear_recurrence.hpp"
#include "fps/fps_light_algorithms.hpp"
#include "convolution/convolution.hpp"
#include "convolution/convolution_light.hpp"
#include "math/modint.hpp"
#include "math/fraction.hpp"
#include <cassert>
#include <climits>
#include <cstdint>
#include <memory>
#include <memory_resource>
#include <random>
#include <type_traits>
#include <utility>

static_assert(std::is_same_v<convolution_mod_detail::DefaultBackend, convolution_mod_detail::FullBackend>);

template <class T>
std::vector<T> naive_convolve(const std::vector<T> &a, const std::vector<T> &b) {
	if (a.empty() || b.empty()) return {};
	std::vector<T> result(a.size() + b.size() - 1, T(0));
	for (std::size_t i = 0; i < a.size(); ++i)
		for (std::size_t j = 0; j < b.size(); ++j) result[i + j] = result[i + j] + a[i] * b[j];
	return result;
}

template <class T>
std::vector<T> modular_convolve(const std::vector<T> &a, const std::vector<T> &b) {
	std::vector<std::uint32_t> x, y;
	for (T value : a) x.push_back(value.val());
	for (T value : b) y.push_back(value.val());
	auto product = convolution_mod(x, y, T::mod());
	std::vector<T> result;
	for (auto value : product) result.push_back(T::raw(value));
	return result;
}

template <class T>
T direct(std::vector<T> initial, const std::vector<T> &coefficients, int k) {
	if (initial.empty()) return T(0);
	std::size_t d = initial.size();
	for (int i = int(d); i <= k; ++i) {
		T value(0);
		for (std::size_t j = 0; j < d; ++j) value = value + coefficients[j] * initial[std::size_t(i) - 1 - j];
		initial.push_back(value);
	}
	return initial[k];
}

template <class T>
std::pair<T, T> fibonacci(std::uint64_t k) {
	if (!k) return {T(0), T(1)};
	auto [a, b] = fibonacci<T>(k / 2);
	T c = a * (T(2) * b - a), d = a * a + b * b;
	return k & 1 ? std::pair<T, T>{d, c + d} : std::pair<T, T>{c, d};
}

template <class T>
void moduli() {
	std::mt19937_64 random(190831);
	for (int d : {0, 1, 2, 3, 8, 17, 33, 64}) {
		std::vector<T> initial, coefficients;
		for (int i = 0; i < d; ++i) {
			initial.push_back(T(random()));
			coefficients.push_back(T(random()));
		}
		for (int k : {0, d / 2, d, d + 1, d + 13, d + 63}) {
			T expected = direct(initial, coefficients, k);
			assert(linear_recurrence(initial, coefficients, k, modular_convolve<T>) == expected);
			if (d <= 8) assert(linear_recurrence(initial, coefficients, k, naive_convolve<T>) == expected);
		}
		for (auto &x : coefficients) x = T(0);
		for (int k : {0, d, d + 13}) assert(linear_recurrence(initial, coefficients, k, modular_convolve<T>) == direct(initial, coefficients, k));
		if (d) coefficients[0] = T(1);
		for (int k : {0, d, d + 13}) assert(linear_recurrence(initial, coefficients, k, modular_convolve<T>) == direct(initial, coefficients, k));
	}
	std::vector<T> initial{T(0), T(1)}, coefficients{T(1), T(1)};
	for (long long k : {1LL << 32, 1000000000000000000LL, LLONG_MAX})
		assert(linear_recurrence(initial, coefficients, k, modular_convolve<T>) == fibonacci<T>(k).first);
}

struct Scalar {
	long long value;
	Scalar() = delete;
	explicit Scalar(long long x) : value(x) {}
	friend Scalar operator+(Scalar a, Scalar b) { return Scalar(a.value + b.value); }
	friend Scalar operator-(Scalar a, Scalar b) { return Scalar(a.value - b.value); }
	friend Scalar operator*(Scalar a, Scalar b) { return Scalar(a.value * b.value); }
};

void scalar_types() {
	std::vector<long long> initial{0, 1}, coefficients{1, 1};
	for (int k = 0; k <= 40; ++k) assert(linear_recurrence(initial, coefficients, k, naive_convolve<long long>) == direct(initial, coefficients, k));
	std::vector<Scalar> x{Scalar(0), Scalar(1)}, c{Scalar(1), Scalar(1)};
	assert(linear_recurrence(x, c, 20, naive_convolve<Scalar>).value == 6765);
	using Q = Fraction<long long>;
	std::vector<Q> a{Q(3, 2)}, r{Q(1, 2)};
	for (int k : {0, 1, 5, 10, 20}) assert(linear_recurrence(a, r, k, naive_convolve<Q>) == direct(a, r, k));
	std::vector<std::uint64_t> u{0, 1}, v{1, 1};
	assert(linear_recurrence(u, v, LLONG_MAX, naive_convolve<std::uint64_t>) == fibonacci<std::uint64_t>(LLONG_MAX).first);
	int calls = 0;
	auto counted = [&](const auto &p, const auto &q) { ++calls; return naive_convolve(p, q); };
	assert(linear_recurrence(initial, coefficients, 1, counted) == 1 && calls == 0);
	assert(linear_recurrence(std::vector<long long>{}, std::vector<long long>{}, LLONG_MAX, counted) == 0 && calls == 0);
	std::pmr::vector<long long> pooled(initial.begin(), initial.end());
	assert(linear_recurrence(pooled, coefficients, 20, counted) == 6765);
	auto move_only = [state = std::make_unique<int>(0)](const auto &p, const auto &q) mutable {
		++*state;
		return naive_convolve(p, q);
	};
	assert(linear_recurrence(initial, coefficients, 20, move_only) == 6765);
	assert(linear_recurrence(initial, coefficients, 20, std::move(move_only)) == 6765);
}

void existing_apis() {
	std::mt19937 random(831991);
	auto full = [](const std::vector<mint> &a, const std::vector<mint> &b) { return convolution(a, b); };
	auto light = [](const std::vector<mint> &a, const std::vector<mint> &b) { return light_convolution::convolution(a, b); };
	for (int d : {0, 1, 2, 8, 33, 64}) {
		std::vector<mint> initial(d), coefficients(d);
		for (auto &x : initial) x = random();
		for (auto &x : coefficients) x = random();
		for (int k : {0, d / 2, d, d + 30, d + 130}) {
			auto expected = linear_recurrence(initial, coefficients, k);
			assert(linear_recurrence(initial, coefficients, k, full) == expected);
			assert(linear_recurrence(initial, coefficients, k, light) == expected);
			assert(light_fps_algorithms::linear_recurrence(initial, coefficients, k) == expected);
		}
	}
}

void constant_arguments() {
	using M = StaticModint<1000000007>;
	std::vector<M> initial(40), coefficients(40);
	for (int i = 0; i < 40; ++i) initial[i] = i + 1, coefficients[i] = i % 7 + 1;
	assert(linear_recurrence(initial, coefficients, 100, modular_convolve<M>) == direct(initial, coefficients, 100));
	assert(linear_recurrence(initial, coefficients, 39, modular_convolve<M>) == M(40));
}

int main() {
	moduli<StaticModint<1>>(); moduli<StaticModint<2>>(); moduli<StaticModint<15>>();
	moduli<StaticModint<998244353>>(); moduli<StaticModint<1000000007>>(); moduli<StaticModint<UINT32_MAX>>();
	for (std::uint32_t modulus : {1U, 2U, 30U, 1000000007U, UINT32_MAX}) {
		DynamicModint<928>::set_mod(modulus);
		moduli<DynamicModint<928>>();
	}
	scalar_types(); existing_apis(); constant_arguments();
}
