#define PROBLEM "https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod"
#define COMB_MAX 9999999
#include "math/combinatorics/comb.hpp"
#include "components/fast_io.hpp"
#include "test/fixtures/comb_modint_types.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int queries = 0;
	unsigned modulus = 0;
	input.read(queries), input.read(modulus);
	using M = comb_test::DynamicField<>;
	M::set_mod(modulus);
	for (int q = 0; q < queries; ++q) {
		int n = 0, k = 0;
		input.read(n), input.read(k);
		output.println(comb<M>(n, k).val());
	}
}
