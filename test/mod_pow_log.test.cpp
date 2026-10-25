#define PROBLEM "https://judge.yosupo.jp/problem/discrete_logarithm_fixed_mod"
#include "math/mod_pow.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::uint32_t p = 0, g = 0;
	int n = 0;
	input.read(p), input.read(g), input.read(n);
	ModPow power(p, g);
	while (n--) {
		std::uint32_t a = 0;
		input.read(a);
		output.println(power.log(a));
	}
}
