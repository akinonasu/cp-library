#include "math/number_theory/factorize.hpp"
#include <functional>
#include <random>

using U = std::uint64_t;
using W = __uint128_t;

void check(U n, const std::vector<U> &expected) {
	assert(divisors(n) == expected);
	auto unordered = divisors(n, false);
	std::sort(unordered.begin(), unordered.end());
	assert(unordered == expected);
	auto factors = factorize_count(n);
	assert(divisors(factors) == expected);
	std::reverse(factors.begin(), factors.end());
	auto reused = divisors(factors, false);
	std::sort(reused.begin(), reused.end());
	assert(reused == expected);
}

int main() {
	constexpr int limit = 20000;
	std::vector<std::vector<U>> expected(limit + 1);
	for (int d = 1; d <= limit; ++d)
		for (int n = d; n <= limit; n += d) expected[n].push_back(d);
	for (int n = 1; n <= limit; ++n) check(n, expected[n]);
	check(12, {1, 2, 3, 4, 6, 12});
	assert(divisors(std::vector<std::pair<U, int>>{}) == std::vector<U>{1});
	for (U p : {2ULL, 3ULL, 5ULL, 37ULL, 65537ULL, 4294967291ULL, 18446744073709551557ULL}) {
		U n = 1;
		std::vector<U> powers{1};
		while (n <= std::numeric_limits<U>::max() / p) {
			n *= p;
			powers.push_back(n);
			check(n, powers);
		}
	}
	check(18446743979220271189ULL, {1, 4294967279ULL, 4294967291ULL, 18446743979220271189ULL});
	std::mt19937_64 rng(20261020);
	for (int trial = 0; trial < 1000; ++trial) {
		U n = rng() % 100000000 + 1;
		std::vector<U> brute;
		for (U d = 1; d <= n / d; ++d) if (n % d == 0) {
			brute.push_back(d);
			if (d != n / d) brute.push_back(n / d);
		}
		std::sort(brute.begin(), brute.end());
		check(n, brute);
	}
	for (U n : {897612484786617600ULL, 18401055938125660800ULL, 18446744073709551615ULL}) {
		auto factors = factorize_count(n);
		std::size_t count = 1;
		W product = 1;
		for (auto [p, e] : factors) {
			count *= e + 1;
			for (int i = 0; i < e; ++i) product *= p;
		}
		assert(product == n);
		auto ds = divisors(n);
		assert(ds.size() == count && ds.front() == 1 && ds.back() == n);
		assert(std::adjacent_find(ds.begin(), ds.end(), std::greater_equal<U>()) == ds.end());
		for (std::size_t i = 0; i < ds.size(); ++i) {
			assert(n % ds[i] == 0);
			assert(W(ds[i]) * ds[ds.size() - i - 1] == n);
		}
		check(n, ds);
	}
}
