#define PROBLEM "https://yukicoder.me/problems/no/2206"
#include "math/combinatorics/binomial_prefix_sum.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int q = 0, limit = 0;
	input.read(q);
	std::vector<std::pair<int, int>> queries(q);
	for (auto& [n, k] : queries) input.read(n), input.read(k), --n, limit = std::max(limit, n);
	BinomialPrefixSum<> prefix(limit);
	for (auto [n, k] : queries) {
		auto total = prefix.query(n, std::int64_t(n) + 1);
		output.println((prefix.query(n, k) * (total + total - mint(1))).val());
	}
}
