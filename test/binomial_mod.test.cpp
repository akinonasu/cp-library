#define PROBLEM "https://judge.yosupo.jp/problem/binomial_coefficient"
#include "math/combinatorics/binomial_mod.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int queries = 0;
	std::uint32_t modulus = 0;
	input.read(queries), input.read(modulus);
	BinomialMod binomial(modulus);
	while (queries--) {
		std::int64_t n = 0, k = 0;
		input.read(n), input.read(k);
		output.println(binomial(n, k));
	}
}
