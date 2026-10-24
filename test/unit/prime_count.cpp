#include "math/number_theory/prime_count.hpp"
#include "math/number_theory/is_prime.hpp"
#include <array>
#include <cassert>
#include <cfenv>
#include <cstdint>
#include <random>
#include <vector>

int main() {
	constexpr int limit = 2000000;
	std::vector<bool> composite(limit + 1);
	std::vector<std::uint32_t> prefix(limit + 1);
	for (int p = 2; p <= limit; ++p) {
		if (!composite[p] && p <= limit / p)
			for (int j = p * p; j <= limit; j += p) composite[j] = true;
		prefix[p] = prefix[p - 1] + !composite[p];
	}
	for (int n = 0; n <= 30000; ++n) assert(prime_count(n) == prefix[n]);
	std::mt19937_64 random(20261022);
	for (int trial = 0; trial < 2000; ++trial) {
		const auto n = random() % (limit + 1);
		assert(prime_count(n) == prefix[n]);
	}
	for (std::uint64_t p = 2; p * p <= limit; ++p)
		for (const auto n : {p * p - 1, p * p, p * p + 1})
			assert(prime_count(n) == prefix[n]);
	for (std::uint64_t p = 2; p * p * p * p <= limit; ++p)
		for (const auto n : {p * p * p * p - 1, p * p * p * p, p * p * p * p + 1})
			assert(prime_count(n) == prefix[n]);
	constexpr std::array<std::uint64_t, 14> powers{
		0, 4, 25, 168, 1229, 9592, 78498, 664579, 5761455,
		50847534, 455052511, 4118054813, 37607912018, 346065536839
	};
	std::uint64_t n = 1;
	for (auto expected : powers) {
		assert(prime_count(n) == expected);
		n *= 10;
	}
	for (const std::uint64_t base : {1000000007ULL, 9999999967ULL, 99999999977ULL, 1000000000039ULL}) {
		std::uint64_t count = 0;
		for (auto x = base; x < base + 256; ++x) count += is_prime(x);
		assert(prime_count(base + 255) - prime_count(base - 1) == count);
	}
	for (std::uint64_t x : {(UINT64_C(1) << 42) - 1, UINT64_C(1) << 42, (UINT64_C(1) << 42) + 1})
		assert(prime_count(x) == prime_count_detail::compute<false>(x));
	const int previous = std::fegetround();
	for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
		if (std::fesetround(mode)) continue;
		for (std::uint64_t x : {0ULL, 1ULL, 2ULL, 3ULL, 9999ULL, 10000ULL, 10001ULL, 999983ULL})
			assert(prime_count(x) == prefix[x]);
		assert(prime_count(100000000000ULL) == 4118054813ULL);
	}
	std::fesetround(previous);
}
