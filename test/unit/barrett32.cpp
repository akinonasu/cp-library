#include "components/barrett32.hpp"
#include <limits>
#include <random>

int main() {
	std::mt19937_64 rng(953772);
	std::uint32_t boundaries[] = {1, 2, 3, 2147483647U, 2147483648U, 2147483649U, 4294967293U, 4294967294U, 4294967295U};
	for (int i = 0; i < 30000; ++i) {
		std::uint32_t mod = i < 9 ? boundaries[i] : std::uint32_t(rng());
		if (!mod) mod = 1;
		Barrett32 m(mod);
		for (int j = 0; j < 8; ++j) {
			std::uint64_t x = j == 0 ? std::numeric_limits<std::uint64_t>::max() : rng();
			assert(m.reduce(x) == x % mod);
			std::uint32_t a = rng(), b = rng();
			assert(m.mul(a, b) == std::uint64_t(a) * b % mod);
		}
		std::uint32_t a = rng(), r = 1 % mod;
		std::uint64_t e = rng(), x = a % mod;
		for (auto k = e; k; k >>= 1, x = x * x % mod) if (k & 1) r = r * x % mod;
		assert(m.pow(a, e) == r);
		assert(m.pow(a, 0) == 1 % mod);
	}
}
