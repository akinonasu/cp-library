#define PROBLEM "https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod"
#include "math/mod_inv.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int queries = 0;
	std::uint32_t modulus = 0;
	input.read(queries), input.read(modulus);
	std::vector<std::pair<int, int>> cases(queries);
	int maximum = 0;
	for (auto &[n, k] : cases) {
		input.read(n), input.read(k);
		maximum = std::max(maximum, n);
	}
	std::vector<std::uint32_t> factorial(maximum + 1, 1);
	for (int n = 1; n <= maximum; ++n)
		factorial[n] = std::uint64_t(factorial[n - 1]) * n % modulus;
	ModInv inverse(modulus);
	for (auto [n, k] : cases) {
		if (k > n) { output.println(0); continue; }
		auto value = std::uint64_t(factorial[n]) * inverse.inv(factorial[k]) % modulus;
		output.println(value * inverse.inv(factorial[n - k]) % modulus);
	}
}
