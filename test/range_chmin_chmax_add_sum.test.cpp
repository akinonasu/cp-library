#define PROBLEM "https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum"
#include "../data_structure/range_chmin_chmax_add_sum.hpp"
#include "../components/fast_io.hpp"
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n);
	in.read(q);
	std::vector<long long> a(n);
	for (auto &x : a) in.read(x);
	RangeChminChmaxAddSum seg(a);
	while (q--) {
		int t = 0, l = 0, r = 0;
		in.read(t);
		in.read(l);
		in.read(r);
		if (t == 3) out.println(seg.sum(l, r));
		else {
			long long x = 0;
			in.read(x);
			if (t == 0) seg.chmin(l, r, x);
			if (t == 1) seg.chmax(l, r, x);
			if (t == 2) seg.add(l, r, x);
		}
	}
}
