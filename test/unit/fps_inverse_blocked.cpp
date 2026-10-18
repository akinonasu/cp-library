#include "fps/fps.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>

static void require(bool value, int line = __builtin_LINE()) {
	if (!value) {
		std::fprintf(stderr, "line %d\n", line);
		std::abort();
	}
}

static void check_inverse(const fps &a, int n) {
	const fps saved = a;
	const fps result = a.inv(n);
	require(result.size() == static_cast<unsigned>(n));
	fps product = (a * result).pre(n);
	product.resize(n);
	if (n) {
		require(product[0] == fps_mint(1));
		product[0] = 0;
	}
	require(product == fps(n));
	for (std::size_t i = 0; i < a.size(); ++i) require(a[i].v == saved[i].v);
}

static void check_root(const fps &a, int n) {
	const fps saved = a;
	const fps result = a.sqrt(n);
	require(result.size() == static_cast<unsigned>(n));
	fps product = (result * result).pre(n), expected = a.pre(n);
	product.resize(n);
	expected.resize(n);
	require(product == expected);
	for (std::size_t i = 0; i < a.size(); ++i) require(a[i].v == saved[i].v);
}

int main() {
	std::mt19937 rng(20261019);
	for (int n : {0, 1, 31, 32, 33, 63, 64, 65, 95, 96, 97, 127, 128, 129, 255, 256, 257, 383, 384, 385, 511, 512, 513, 767, 768, 769, 1023, 1024, 1025, 4095, 4096, 4097, 65537}) {
		for (int input : {1, 2, 31, 33, 63, 64, 65, n / 3 + 1, n + 17}) {
			fps a(input);
			for (auto &x : a) x = fps_mint::word(2 * fps_mint::mod() - 1 - rng() % 32);
			a[0] = 1;
			check_inverse(a, n);
			check_root(a, n);
			if (n <= 1025) {
				a.insert(a.begin(), 2, fps_mint(0));
				check_root(a, n + 2);
				a.insert(a.begin(), 1, fps_mint(0));
				require(a.sqrt(n + 4).empty());
			}
		}
	}
	fps a(500000);
	for (auto &x : a) x = rng();
	a[0] = 1;
	check_inverse(a, a.size());
	check_root(a, a.size());
	for (int n : {0, 1, 255, 256, 257, 511, 512, 513, 4097}) {
		require(fps{}.sqrt(n) == fps(n));
		require(fps{0, 0, 4}.sqrt(n).size() == static_cast<unsigned>(n));
		require(fps{3}.sqrt(n).empty() || n == 0);
	}
}
