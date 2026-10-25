#include "convolution/gcd_lcm_convolution.hpp"
#include "math/mint.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

template <bool gcd, class T>
std::vector<T> direct(std::vector<T> a, std::vector<T> b) {
	if (a.empty() || b.empty()) return {};
	const std::size_t n = std::max(a.size(), b.size());
	a.resize(n), b.resize(n);
	std::vector<T> x(n), y(n);
	for (std::size_t i = 1; i < n; ++i)
		for (std::size_t j = i; j < n; j += i) {
			x[gcd ? i : j] += a[gcd ? j : i];
			y[gcd ? i : j] += b[gcd ? j : i];
		}
	for (std::size_t i = 1; i < n; ++i) x[i] *= y[i];
	for (std::size_t step = 1; step < n; ++step) {
		const std::size_t i = gcd ? n - step : step;
		for (std::size_t j = i + i; j < n; j += i) x[gcd ? i : j] -= x[gcd ? j : i];
	}
	return x;
}

template <class T>
void check(const std::vector<T> &a, const std::vector<T> &b) {
	assert(gcd_convolution(a, b) == direct<true>(a, b));
	assert(lcm_convolution(a, b) == direct<false>(a, b));
}

int main() {
	std::mt19937 rng(672045);
	check(std::vector<mint>{}, std::vector<mint>{0, 1});
	check(std::vector<mint>{0}, std::vector<mint>{});
	check(std::vector<mint>{0}, std::vector<mint>{0});
	for (int n : {2, 3, 4, 7, 8, 9, 31, 127, 1024, 4095, 4096, 4097, 4225, 4226, 10000, 10001, 10002, 10007, 65536, 65537, 65538}) {
		std::vector<mint> a(n), b(n - n / 3);
		for (int trial = 0; trial < 4; ++trial) {
			for (std::size_t i = 1; i < a.size(); ++i) a[i] = mint::word(trial ? rng() % (2 * mint::mod()) : 2 * mint::mod() - 1);
			for (std::size_t i = 1; i < b.size(); ++i) b[i] = mint::word(trial ? rng() % (2 * mint::mod()) : mint::mod());
			check(a, b);
		}
	}
	for (int trial = 0; trial < 50; ++trial) {
		std::vector<std::int64_t> a(1 + rng() % 500), b(1 + rng() % 500);
		for (std::size_t i = 1; i < a.size(); ++i) a[i] = std::int64_t(rng() % 201) - 100;
		for (std::size_t i = 1; i < b.size(); ++i) b[i] = std::int64_t(rng() % 201) - 100;
		check(a, b);
	}
}
