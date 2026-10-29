#define PROBLEM "https://judge.yosupo.jp/problem/prefix_sum_of_binomial_coefficient"
#include "../math/binomial_prefix_sum_light.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	std::vector<std::pair<int, int>> queries(t);
	for (auto& [n, k] : queries) {
		input.read(n);
		input.read(k);
		++k;
	}
	for (auto value : light_combinatorics::binomial_prefix_sum(queries)) output.println(value.val());
}
