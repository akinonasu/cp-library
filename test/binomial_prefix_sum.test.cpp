#define PROBLEM "https://judge.yosupo.jp/problem/prefix_sum_of_binomial_coefficient"
#include "../math/binomial_prefix_sum.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	BinomialPrefixSum<> prefix(200000);
	while (t--) {
		int n = 0, k = 0;
		input.read(n);
		input.read(k);
		output.println(prefix.query(n, std::int64_t(k) + 1).val());
	}
}
