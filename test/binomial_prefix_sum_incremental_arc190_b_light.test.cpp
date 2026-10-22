#define PROBLEM "https://atcoder.jp/contests/arc190/tasks/arc190_b"
#include "../math/combinatorics/binomial_prefix_sum_incremental_light.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n, a, b, q;
	in.read(n), in.read(a), in.read(b), in.read(q);
	--a, --b;
	light_combinatorics::IncrementalBinomialPrefixSum<> left_a(n);
	auto right_a = left_a, left_b = left_a, right_b = left_a;
	auto cumulative = [&](int k) -> mint {
		if (!k) return mint(0);
		int t = n - k;
		mint x = left_a.query(t, a + 1) - right_a.query(t, a - k + 1);
		mint y = left_b.query(t, b + 1) - right_b.query(t, b - k + 1);
		return x * y * mint(4).pow(k - 1);
	};
	int last = 0;
	mint previous(0);
	while (q--) {
		int k;
		in.read(k);
		mint lower = last == k - 1 ? previous : cumulative(k - 1);
		mint upper = cumulative(k);
		out.println((upper - lower).val());
		last = k, previous = upper;
	}
}
