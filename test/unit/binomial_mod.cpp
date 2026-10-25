#include "math/combinatorics/binomial_mod.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <random>

std::uint32_t exact(std::int64_t n, int k, std::uint32_t m) {
	boost::multiprecision::cpp_int x = 1;
	for (int i = 1; i <= k; ++i) x = x * (n - i + 1) / i;
	return (x % m).convert_to<std::uint32_t>();
}

int main() {
	for (unsigned m = 1; m <= 400; ++m) {
		BinomialMod full(m), partial(m, 90);
		assert(full.mod() == m && full.max_n() == INT64_MAX);
		std::vector<unsigned> row(1, 1 % m);
		for (int n = 0; n <= 90; ++n) {
			for (int k = -1; k <= n + 1; ++k) {
				unsigned expected = k < 0 || n < k ? 0 : row[k];
				assert(full(n, k) == expected && partial.comb(n, k) == expected);
			}
			row.push_back(0);
			for (int k = n + 1; k; --k) row[k] = (row[k] + row[k - 1]) % m;
		}
		assert(full(-1, 0) == 0 && full(2, -1) == 0);
	}
	std::mt19937_64 rng(765636);
	for (std::uint32_t m : {1U, 2U, 4U, 8U, 9U, 16U, 27U, 72U, 256U, 729U, 65536U, 510510U, 524288U, 720720U, 999983U}) {
		BinomialMod comb(m);
		for (int trial = 0; trial < 100; ++trial) {
			std::int64_t n = std::int64_t(rng() & INT64_MAX);
			int k = rng() % 80;
			assert(comb(n, k) == exact(n, k, m));
			assert(comb(n, k) == comb(n, n - k));
			if (k) assert(comb(n, k) == (std::uint64_t(comb(n - 1, k)) + comb(n - 1, k - 1)) % m);
		}
	}
	for (std::uint32_t m : {1000000007U, 998244353U, 2147483648U, 3221225472U, 3486784401U, 4294967291U, 4294967295U}) {
		BinomialMod comb(m, 250);
		assert(comb.max_n() == 250);
		for (int n = 0; n <= 250; ++n) for (int k = 0; k <= n; ++k) assert(comb(n, k) == exact(n, k, m));
		auto copy = comb;
		assert(copy(250, 100) == comb(250, 100));
		bool caught = false;
		try { comb(251, 1); } catch (const std::out_of_range &) { caught = true; }
		assert(caught && comb(251, -1) == 0);
	}
	for (unsigned m : {1U, 2U, 4U, 9U, 100U}) {
		BinomialMod comb(m, 0);
		assert(comb(0, 0) == 1 % m);
	}
	bool caught = false;
	try { BinomialMod invalid(0); } catch (const std::invalid_argument &) { caught = true; }
	assert(caught);
}
