#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_primes"
#include "math/number_theory/prime_sieve.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::size_t n = 0, a = 0, b = 0;
	input.read(n), input.read(a), input.read(b);
	std::vector<std::size_t> selected;
	std::size_t count = 0, next = b;
	for_each_prime(n + 1, [&](std::size_t p) {
		if (count == next) selected.push_back(p), next += a;
		++count;
	});
	output.write(count), output.write(' '), output.println(selected.size());
	for (std::size_t i = 0; i < selected.size(); ++i) {
		if (i) output.write(' ');
		output.write(selected[i]);
	}
	output.write('\n');
}
