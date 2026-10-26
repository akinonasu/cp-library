#define PROBLEM "https://judge.yosupo.jp/problem/primality_test"
#include "math/is_prime_light.hpp"
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
		output.println(light_is_prime(n) ? "Yes" : "No");
	}
}
