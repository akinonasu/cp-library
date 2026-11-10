#include "convolution/multivariate_convolution.hpp"
#include "convolution/multivariate_convolution_light.hpp"
#include "math/modint.hpp"
#include <cassert>
#include <numeric>
#include <random>

int main() {
	std::mt19937 rng(146613);
	std::vector<std::vector<int>> shapes{{}, {1}, {1, 7, 1}, {2, 3}, {7, 2}, {2, 7}, {2, 2, 2, 3}, {3, 2, 3, 2, 2}, {5, 3, 2, 4}, {2, 2, 2, 2, 2, 2, 2}, {3, 3, 3, 3, 3}, {2, 3, 2, 3, 2, 3}};
	for (int t = 0; t < 40; ++t) {
		std::vector<int> dims;
		int n = 1;
		for (int k = 1 + rng() % 7; k--;) {
			int d = 1 + rng() % 7;
			if (n * d <= 400) dims.push_back(d), n *= d;
		}
		shapes.push_back(dims);
	}
	for (const auto &dims : shapes) {
		int n = std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<int>{});
		std::vector<mint> a(n), b(n), expected(n);
		for (auto *v : {&a, &b}) for (auto &x : *v) x = mint::word(rng() % (2 * mint::mod()));
		a[0] = mint::word(mint::mod());
		b.back() = mint::word(2 * mint::mod() - 1);
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
			bool valid = true;
			int u = i, v = j;
			for (int d : dims) {
				valid &= u % d + v % d < d;
				u /= d, v /= d;
			}
			if (valid) expected[i + j] += a[i] * b[j];
		}
		const auto saved_a = a, saved_b = b;
		assert(multivariate_convolution(a, b, dims) == expected);
		assert(light_convolution::multivariate_convolution(a, b, dims) == expected);
		for (int i = 0; i < n; ++i) assert(a[i].v == saved_a[i].v && b[i].v == saved_b[i].v);
		using OtherMint = StaticModint<998244353>;
		std::vector<OtherMint> x(n), y(n), result(n);
		for (int i = 0; i < n; ++i) x[i] = a[i].val(), y[i] = b[i].val(), result[i] = expected[i].val();
		assert(multivariate_convolution(x, y, dims) == result);
		assert(light_convolution::multivariate_convolution(x, y, dims) == result);
	}
}
