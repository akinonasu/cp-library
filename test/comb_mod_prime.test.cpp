#define PROBLEM "https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod"
#include "math/comb_mod.hpp"
#include "components/fast_io.hpp"
#include "fixtures/comb_modint_types.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int queries = 0;
	unsigned modulus = 0;
	input.read(queries), input.read(modulus);
	using Mint = comb_test::DynamicField<>;
	Mint::set_mod(modulus);
	std::vector<std::pair<int, int>> cases(queries);
	int maximum = 0;
	for (auto &[n, k] : cases) {
		input.read(n), input.read(k);
		maximum = std::max(maximum, n);
	}
	auto [choose, factorial, inverse_factorial] = make_comb<Mint>(maximum);
	for (auto [n, k] : cases) {
		output.println(choose(n, k).val());
	}
}
