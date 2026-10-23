#define PROBLEM "https://judge.yosupo.jp/problem/primality_test"
#include "math/number_theory/is_prime.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	while (q--) {
		std::uint64_t n = 0;
		input.read(n);
		output.println(is_prime(n) ? "Yes" : "No");
	}
}
