#define PROBLEM "https://judge.yosupo.jp/problem/sum_of_multiplicative_function"
#include "math/number_theory/multiplicative_prefix_sum.hpp"
#include "components/fast_io.hpp"
#include "test/fixtures/multiplicative_prefix_sum_types.hpp"

int main() {
	using M = MultiplicativeSumModint<469762049>;
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint64_t n = 0;
		unsigned a = 0, b = 0;
		input.read(n), input.read(a), input.read(b);
		MultiplicativePrefixSum<M> sieve(n);
		auto sum = sieve.prime_sum_linear(M(a), M(b));
		const auto value = sieve.sum(sum, [a, b](std::uint64_t p, unsigned e, std::uint64_t) { return M(a) * M(e) + M(b) * M(p); });
		output.println(value.val());
	}
}
