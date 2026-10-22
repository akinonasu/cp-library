#define COMB_MAX 70000
#include "math/combinatorics/comb.hpp"
#include <cstdint>
#include <cstdlib>
#include <vector>

void require(bool value) { if (!value) std::abort(); }

int main() {
	std::uint64_t automatic = 1;
	for (int i = 0; i <= COMB_INF; ++i) {
		if (i) automatic = automatic * i % mint::mod();
		require(fact[i].val() == automatic && fact[i] * finv[i] == mint(1));
		require(comb(i, 0) == mint(1) && comb(i, i) == mint(1));
		if (i) require(comb(i, 1) == mint(i));
	}
	mint_detail::factorials(nullptr, nullptr, 0);
	for (int n : {0, 1, 2, 7, 8, 31, 32, 63, 64, 127, 128, 255, 256, 257, 511, 512, 513, 65535, 65536, 65537, 1048576}) {
		std::vector<mint> a(n + 2, mint(17)), b(n + 2, mint(19));
		mint_detail::factorials(a.data() + 1, b.data() + 1, n);
		require(a.front() == mint(17) && a.back() == mint(17));
		require(b.front() == mint(19) && b.back() == mint(19));
		std::uint64_t expected = 1;
		for (int i = 0; i < n; ++i) {
			if (i) expected = expected * i % mint::mod();
			require(a[i + 1].val() == expected && a[i + 1] * b[i + 1] == mint(1));
		}
	}
}
