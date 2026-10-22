#define PROBLEM "https://judge.yosupo.jp/problem/sum_of_totient_function"
#include "math/number_theory/multiplicative_prefix_sum.hpp"
#include "math/mint.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::uint64_t n = 0;
	input.read(n);
	MultiplicativePrefixSum<mint> sieve(n);
	auto sum = sieve.prime_sum_linear(mint(-1), mint(1));
	auto answer = sieve.sum(sum, [](std::uint64_t p, unsigned, std::uint64_t power) { return mint(power - power / p); });
	output.println(answer.val());
}
