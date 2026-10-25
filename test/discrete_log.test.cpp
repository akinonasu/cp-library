#define PROBLEM "https://judge.yosupo.jp/problem/discrete_logarithm_mod"
#include "math/discrete_log.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint32_t a = 0, b = 0, mod = 0;
		input.read(a), input.read(b), input.read(mod);
		auto result = discrete_log(a, b, mod);
		if (result) output.println(*result);
		else output.println(-1);
	}
}
