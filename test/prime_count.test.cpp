#define PROBLEM "https://judge.yosupo.jp/problem/counting_primes"
#include "math/number_theory/prime_count.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::uint64_t n = 0;
	input.read(n);
	output.println(prime_count(n));
}
