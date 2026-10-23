#define PROBLEM "https://judge.yosupo.jp/problem/rectangle_sum"
#include "../data_structure/persistent/persistent_segtree.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<std::array<int, 3>> points(n);
	std::vector<int> xs(n), ys(n);
	for (int i = 0; i < n; ++i) {
		std::cin >> points[i][0] >> points[i][1] >> points[i][2];
		ys[i] = points[i][1];
	}
	std::sort(points.begin(), points.end());
	std::sort(ys.begin(), ys.end());
	ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
	PersistentSegtree<long long, op, e> tree(static_cast<int>(ys.size()));
	std::vector<decltype(tree)::root_type> roots(n + 1, tree.initial_root());
	for (int i = 0; i < n; ++i) {
		xs[i] = points[i][0];
		int p = std::lower_bound(ys.begin(), ys.end(), points[i][1]) - ys.begin();
		roots[i + 1] = tree.multiply(roots[i], p, points[i][2]);
	}
	while (q--) {
		int l, d, r, u;
		std::cin >> l >> d >> r >> u;
		int a = std::lower_bound(xs.begin(), xs.end(), l) - xs.begin();
		int b = std::lower_bound(xs.begin(), xs.end(), r) - xs.begin();
		int first = std::lower_bound(ys.begin(), ys.end(), d) - ys.begin();
		int last = std::lower_bound(ys.begin(), ys.end(), u) - ys.begin();
		std::cout << tree.prod(roots[b], first, last) - tree.prod(roots[a], first, last) << '\n';
	}
}
