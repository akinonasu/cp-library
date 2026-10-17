#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_range_contour_sum_on_tree"
#include "graph/centroid_decomposition.hpp"
#include <cstdint>
#include <iostream>

int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);
	int n, q;
	std::cin >> n >> q;
	std::vector<std::int64_t> values(n);
	for (auto& x : values) std::cin >> x;
	CentroidDecomposition<> tree(n);
	for (int i = 1, u, v; i < n; ++i) {
		std::cin >> u >> v;
		tree.add_edge(u, v);
	}
	tree.build();
	std::vector<int> length(2 * n);
	for (int v = 0; v < n; ++v) tree.for_each_ancestor(v, [&](int c, int d, int b) {
		length[c] = std::max(length[c], d + 1);
		if (b != -1) length[n + b] = std::max(length[n + b], d + 1);
	});
	std::vector<std::size_t> offset(2 * n + 1);
	for (int i = 0; i < 2 * n; ++i) offset[i + 1] = offset[i] + length[i];
	std::vector<std::int64_t> bit(offset.back());
	for (int v = 0; v < n; ++v) tree.for_each_ancestor(v, [&](int c, int d, int b) {
		bit[offset[c] + d] += values[v];
		if (b != -1) bit[offset[n + b] + d] += values[v];
	});
	for (int g = 0; g < 2 * n; ++g)
		for (int i = 1; i <= length[g]; ++i) {
			int j = i + (i & -i);
			if (j <= length[g]) bit[offset[g] + j - 1] += bit[offset[g] + i - 1];
		}
	auto add = [&](int g, int d, std::int64_t x) {
		for (int i = d + 1; i <= length[g]; i += i & -i) bit[offset[g] + i - 1] += x;
	};
	auto prefix = [&](int g, int r) {
		std::int64_t result = 0;
		for (int i = std::min(r, length[g]); i > 0; i -= i & -i) result += bit[offset[g] + i - 1];
		return result;
	};
	while (q--) {
		int type, v;
		std::cin >> type >> v;
		if (type == 0) {
			std::int64_t x;
			std::cin >> x;
			tree.for_each_ancestor(v, [&](int c, int d, int b) {
				add(c, d, x);
				if (b != -1) add(n + b, d, x);
			});
		} else {
			int l, r;
			std::cin >> l >> r;
			std::int64_t answer = 0;
			tree.for_each_ancestor(v, [&](int c, int d, int b) {
				answer += prefix(c, r - d) - prefix(c, l - d);
				if (b != -1) answer -= prefix(n + b, r - d) - prefix(n + b, l - d);
			});
			std::cout << answer << '\n';
		}
	}
}
