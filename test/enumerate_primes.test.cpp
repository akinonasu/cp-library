#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_primes"
#include "math/number_theory/prime_sieve.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::size_t n = 0, a = 0, b = 0;
	input.read(n), input.read(a), input.read(b);
	const auto primes = prime_sieve(n + 1);
	const auto count = primes.size() > b ? (primes.size() - b - 1) / a + 1 : 0;
	output.write(primes.size()), output.write(' '), output.println(count);
	for (std::size_t i = b; i < primes.size(); i += a) {
		if (i != b) output.write(' ');
		output.write(primes[i]);
	}
	output.write('\n');
}
