#define COMB_MAX 200
#include "math/is_prime_light.hpp"
#include "math/comb_light.hpp"
#include "math/binomial_prefix_sum_light.hpp"
#include "math/binomial_prefix_sum_incremental_light.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <type_traits>

static_assert(std::is_same_v<Comb<>, light_combinatorics::Comb<>>);
static_assert(std::is_same_v<BinomialPrefixSum<>, light_combinatorics::BinomialPrefixSum<>>);
static_assert(std::is_same_v<IncrementalBinomialPrefixSum<>, light_combinatorics::IncrementalBinomialPrefixSum<>>);

int main() {
	bool (*prime)(std::uint64_t) = is_prime;
	for (int n = 0; n < 10000; ++n) {
		bool expected = n >= 2;
		for (int d = 2; d * d <= n; ++d) if (n % d == 0) expected = false;
		assert(prime(n) == expected);
	}
	assert(is_prime(18446744073709551557ULL));
	assert(!is_prime(18446744073709551615ULL));
	assert(COMB_INF == COMB_MAX && fact[-1] == mint(0) && finv[-1] == mint(0));
	std::array<mint, COMB_MAX + 1> row{};
	row[0] = 1;
	for (int n = 0; n <= COMB_MAX; ++n) {
		assert(comb(n, -1) == mint(0) && comb(n, n + 1) == mint(0));
		assert(fact[n] * finv[n] == mint(1));
		for (int k = 0; k <= n; ++k) {
			assert(comb(n, k) == row[k]);
			assert(comb<mint>(n, k) == row[k]);
			assert(Comb<>::comb(n, k) == row[k]);
		}
		for (int k = std::min(n + 1, COMB_MAX); k > 0; --k) row[k] += row[k - 1];
	}
	BinomialPrefixSum online(100);
	IncrementalBinomialPrefixSum incremental(100);
	std::vector<std::pair<int, std::int64_t>> queries;
	std::vector<mint> answers;
	for (int n = 0; n <= 100; ++n) {
		mint sum = 0;
		for (int k = 0; k <= n + 2; ++k) {
			queries.emplace_back(n, k);
			answers.push_back(sum);
			assert(online.query(n, k) == sum);
			assert(incremental.query(n, k) == sum);
			sum += comb(n, k);
		}
	}
	assert(binomial_prefix_sum(queries) == answers);
	assert(binomial_prefix_sum<mint>(queries, 7) == answers);
}
