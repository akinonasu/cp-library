#define PROBLEM "https://judge.yosupo.jp/problem/min_of_mod_of_linear"
#include "math/floor_sum/min_of_mod_of_linear.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint32_t n = 0, m = 0, a = 0, b = 0;
		input.read(n), input.read(m), input.read(a), input.read(b);
		output.println(std::uint32_t(min_of_mod_of_linear(n, m, a, b)));
	}
}
