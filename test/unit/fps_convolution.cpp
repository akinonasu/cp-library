#include "convolution/convolution.hpp"
#include <cstdlib>
#include <random>

static void require(bool value) { if (!value) std::abort(); }

int main() {
	std::mt19937 rng(20261019);
	for (int n : {33, 64, 65, 128, 129, 255, 256, 257, 511, 512, 513, 1024}) {
		for (int m : {33, 64, 129, 256, 512}) {
			fps a(n), b(m), expected(n + m - 1);
			for (auto &x : a) x = mint::word(rng() % (2 * mint::mod()));
			for (auto &x : b) x = mint::word(rng() % (2 * mint::mod()));
			const fps saved_a = a, saved_b = b;
			for (int i = 0; i < n; ++i)
				for (int j = 0; j < m; ++j) expected[i + j] += a[i] * b[j];
			require(a * b == expected);
			require(convolution(static_cast<const std::vector<mint> &>(a), static_cast<const std::vector<mint> &>(b)) == static_cast<const std::vector<mint> &>(expected));
			require(a == saved_a && b == saved_b);
		}
	}
	for (int n : {33, 65, 256, 513, 4096, 32769, 524288}) {
		fps a(n), expected(2 * n - 1);
		const int positions[] = {0, 1, n / 2, n - 2, n - 1};
		for (int i : positions) a[i] = rng();
		for (int i : positions)
			for (int j : positions) expected[i + j] += a[i] * a[j];
		require(a * a == expected);
		require(convolution(static_cast<const std::vector<mint> &>(a), static_cast<const std::vector<mint> &>(a)) == static_cast<const std::vector<mint> &>(expected));
	}
	const fps empty;
	for (int n : {33, 65, 256, 513, 4096, 32769, 131072}) {
		fps a(n);
		for (auto &x : a) x = mint::word(rng() % (2 * mint::mod()));
		const fps saved = a, b = a;
		const fps expected = a * b;
		require(a * a == expected);
		require(convolution(static_cast<const std::vector<mint> &>(a), static_cast<const std::vector<mint> &>(a)) == static_cast<const std::vector<mint> &>(expected));
		require(a == saved);
	}
	require((empty * fps{1}).empty());
}
