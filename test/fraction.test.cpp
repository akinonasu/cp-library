#define PROBLEM "https://judge.yosupo.jp/problem/rational_approximation"
#include "math/number_theory/stern_brocot_tree.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint32_t n = 0, x = 0, y = 0;
		input.read(n), input.read(x), input.read(y);
		auto [lower, upper] = SternBrocotTree<std::uint32_t>::approximate(x, y, n);
		output.write(lower.first), output.write(' '), output.write(lower.second), output.write(' ');
		output.write(upper.first), output.write(' '), output.println(upper.second);
	}
}
