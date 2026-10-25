#include "convolution/multiplicative_convolution.hpp"
#include "convolution/multiplicative_convolution_light.hpp"
#include <cassert>
#include <random>

int main() {
	std::mt19937 rng(23409);
	for (int p : {2, 3, 17, 97, 257, 263, 509}) {
		std::vector<mint> a(p), b(p), expected(p), square(p);
		for (auto &x : a) x = mint::word(rng() % (2 * mint::mod()));
		for (auto &x : b) x = mint::word(rng() % (2 * mint::mod()));
		const auto saved = a;
		for (int i = 0; i < p; ++i) for (int j = 0; j < p; ++j) {
			expected[i * j % p] += a[i] * b[j];
			square[i * j % p] += a[i] * a[j];
		}
		assert(multiplicative_convolution(a, b) == expected);
		assert(light_convolution::multiplicative_convolution(a, b) == expected);
		assert(multiplicative_convolution(a, a) == square);
		assert(light_convolution::multiplicative_convolution(a, a) == square);
		assert(a == saved);
	}
	for (int p : {65537, 131071}) {
		std::vector<mint> a(p), expected(p);
		const int positions[] = {0, 1, 2, p / 2, p - 1};
		for (int i : positions) a[i] = rng();
		for (int i : positions) for (int j : positions) expected[std::uint64_t(i) * j % p] += a[i] * a[j];
		assert(multiplicative_convolution(a, a) == expected);
		assert(light_convolution::multiplicative_convolution(a, a) == expected);
	}
}
