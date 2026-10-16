#define PROBLEM "https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum"
#include "../data_structure/segtree_beats.hpp"
#include <iostream>
#include <vector>

int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);
	int n, q;
	std::cin >> n >> q;
	std::vector<long long> a(n);
	for (auto &x : a) std::cin >> x;
	SegtreeBeats seg(a);
	while (q--) {
		int t, l, r;
		std::cin >> t >> l >> r;
		if (t == 3) std::cout << seg.sum(l, r) << '\n';
		else {
			long long x;
			std::cin >> x;
			if (t == 0) seg.chmin(l, r, x);
			if (t == 1) seg.chmax(l, r, x);
			if (t == 2) seg.add(l, r, x);
		}
	}
}
