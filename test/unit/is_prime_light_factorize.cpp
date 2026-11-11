#include "math/is_prime_light.hpp"
#include "math/factorize.hpp"
#include <array>
#include <cassert>

int main() {
	for (std::uint64_t n : std::array<std::uint64_t, 6>{1ULL, 2ULL, 3ULL, 1000000007ULL, 1000000007ULL * 1000000009ULL,
		std::numeric_limits<std::uint64_t>::max()}) {
		auto factors = factorize(n);
		std::uint64_t product = 1;
		for (auto p : factors) {
			assert(is_prime(p));
			product *= p;
		}
		assert(product == n);
	}
}
