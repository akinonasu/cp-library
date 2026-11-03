#include "math/factorial_mod.hpp"
#include <cassert>
#include <limits>
#include <random>
int main() {
	constexpr std::uint64_t mod = 998244353;
	std::uint64_t expected = 1;
	for (unsigned n = 0; n <= 100000; ++n) {
		if (n) expected = expected * n % mod;
		if (n < 100 || n % 997 == 0 || n % 65536 < 3 || n % 65536 > 65533) assert(factorial_mod(n).val() == expected);
	}
	assert(factorial_mod(0) == mint(1));
	assert(factorial_mod(mod - 1) == mint(-1));
	assert(factorial_mod(mod - 2) == mint(1));
	assert(factorial_mod(mod) == mint(0) && factorial_mod(mod + 1) == mint(0));
	assert(factorial_mod(std::numeric_limits<std::uint64_t>::max()) == mint(0));
	std::mt19937 rng(812345);
	for (int k = 0; k < 180; ++k) {
		unsigned n = rng() % (mod - 2);
		mint f = factorial_mod(n), reflected = factorial_mod(mod - 1 - n);
		assert(f * reflected == (n % 2 ? mint(1) : mint(-1)));
		assert(factorial_mod(n + 1) == f * mint(n + 1));
	}
	for (unsigned block : {1U, 2U, 70U, 7616U, 15231U}) {
		unsigned anchor = block * 65536;
		mint value = factorial_mod_detail::table[block];
		assert(factorial_mod(anchor) == value);
		assert(factorial_mod(anchor + 1) == value * mint(anchor + 1));
		assert(factorial_mod(anchor - 1) * mint(anchor) == value);
		assert(factorial_mod(anchor + 32768) == factorial_mod(anchor + 32767) * mint(anchor + 32768));
	}
}
