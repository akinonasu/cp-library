#define PROBLEM "https://yukicoder.me/problems/no/2206"
#include "math/binomial_prefix_sum_light.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int q = 0, limit = 0;
	input.read(q);
	std::vector<std::pair<int, int>> queries(q);
	for (auto& [n, k] : queries) input.read(n), input.read(k), --n, limit = std::max(limit, n);
	auto answers = light_combinatorics::binomial_prefix_sum(queries);
	std::vector<mint> power2(limit + 2, mint(1));
	for (int i = 1; i < int(power2.size()); ++i) power2[i] = power2[i - 1] + power2[i - 1];
	for (int i = 0; i < q; ++i) output.println((answers[i] * (power2[queries[i].first + 1] - mint(1))).val());
}
