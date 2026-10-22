#include "math/number_theory/multiplicative_prefix_sum.hpp"
#include "multiplicative_prefix_sum_types.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
	if (argc != 3) return 1;
	using M = MultiplicativeSumModint<469762049>;
	const auto n = std::strtoull(argv[2], nullptr, 10);
	const auto start = std::chrono::steady_clock::now();
	MultiplicativePrefixSum<M> sieve(n);
	const auto initialized = std::chrono::steady_clock::now();
	auto sum = sieve.prime_sum_linear(M(7), M(11));
	const auto primes = std::chrono::steady_clock::now();
	auto power = [](std::uint64_t p, unsigned e, std::uint64_t) { return M(7) * M(e) + M(11) * M(p); };
	const auto answer = argv[1][0] == 's' ? sieve.sum(sum, power) : sieve.get(sieve.prefix_sum(sum, power), n);
	const auto stop = std::chrono::steady_clock::now();
	std::printf("%u %.6f %.6f %.6f\n", answer.val(),
		std::chrono::duration<double, std::milli>(initialized - start).count(),
		std::chrono::duration<double, std::milli>(primes - initialized).count(),
		std::chrono::duration<double, std::milli>(stop - primes).count());
}
