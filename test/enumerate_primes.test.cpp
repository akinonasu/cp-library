#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_primes"
#include "math/prime_sieve.hpp"
#include "components/fast_io_output_values.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::size_t n = 0, a = 0, b = 0;
	input.read(n), input.read(a), input.read(b);
	auto [count, selected] = prime_sieve_sample(n + 1, a, b);
	output.write(count), output.write(' '), output.println(selected.size());
	if (selected.empty()) output.write('\n');
	else write_unsigned_values(output, selected.size(), [&](std::size_t i) { return selected[i]; });
}
