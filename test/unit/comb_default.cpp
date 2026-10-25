#include "math/comb.hpp"
#include <cstdint>
#include <cstdlib>
#include <limits>

static_assert(COMB_INF == 10000000);
const unsigned before_main = fact[5].val();

void require(bool value) { if (!value) std::abort(); }

int main() {
	require(before_main == 120);
	std::uint64_t expected = 1;
	for (int i = 0; i <= COMB_INF; ++i) {
		if (i) expected = expected * i % mint::mod();
		require(fact[i].val() == expected);
		require(fact[i] * finv[i] == mint(1));
	}
	require(comb(COMB_INF, 0) == mint(1) && comb(COMB_INF, COMB_INF) == mint(1));
	require(comb(COMB_INF, 1) == mint(COMB_INF));
	require(comb(COMB_INF, 2) == mint(std::uint64_t(COMB_INF) * (COMB_INF - 1) / 2));
	require(fact[std::numeric_limits<int>::min()] == mint(0) && finv[-1] == mint(0));
}
