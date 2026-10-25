#define PROBLEM "https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum"
#include "fixtures/beats.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	std::vector<clamp_beats::S> a(n);
	for (auto &node : a) {
		long long value = 0;
		in.read(value);
		node = clamp_beats::leaf(value);
	}
	clamp_beats::Tree seg(a);
	while (q--) {
		int t = 0, l = 0, r = 0;
		in.read(t), in.read(l), in.read(r);
		if (t == 3) out.println(seg.prod(l, r).sum);
		else {
			long long x = 0;
			in.read(x);
			auto f = clamp_beats::id();
			if (t == 0) f.hi = x;
			if (t == 1) f.lo = x;
			if (t == 2) f.add = x;
			seg.apply(l, r, f);
		}
	}
}
