#include "math/combinatorics/binomial_prefix_sum_incremental.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n, a, b, q;
	in.read(n), in.read(a), in.read(b);
	--a, --b;
	IncrementalBinomialPrefixSum<> pre1(n);
	auto pre2 = pre1;
	std::vector<mint> ans(n + 1);
	auto comb = [&](int row, int k) { return pre1.binom(row, k); };
	for (int k = 1; k <= n; ++k) ans[k] += comb(n - k, a) * (pre1.query(n - k, b + 1) - pre2.query(n - k, b - k + 1));
	for (int k = n; k >= 1; --k) ans[k] += comb(n - k, b - k + 1) * (pre1.query(n - k, a + 1) - pre2.query(n - k, a - k + 1));
	for (int k = 1; k <= n; ++k) ans[k] += comb(n - k, b) * (pre1.query(n - k, a + 1) - pre2.query(n - k, a - k + 1));
	for (int k = n; k >= 1; --k) ans[k] += comb(n - k, a - k + 1) * (pre1.query(n - k, b + 1) - pre2.query(n - k, b - k + 1));
	mint power = mint(4).inv();
	for (int k = 1; k <= n; ++k) {
		mint x = comb(n - k, a) + comb(n - k, a - k + 1);
		mint y = comb(n - k, b) + comb(n - k, b - k + 1);
		ans[k] = (ans[k] + ans[k] - x * y) * power;
		power *= mint(4);
	}
	in.read(q);
	while (q--) {
		int k;
		in.read(k);
		out.println(ans[k].val());
	}
}
