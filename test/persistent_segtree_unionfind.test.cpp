#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#include "../data_structure/persistent_segtree.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

int op(int a, int b) { return std::max(a, b); }
int e() { return std::numeric_limits<int>::min(); }

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	PersistentSegtree<int, op, e> tree(std::vector<int>(n, -1));
	std::vector<decltype(tree)::root_type> roots(q + 1, tree.initial_root());
	auto find = [&](auto root, int x) {
		int parent;
		while ((parent = tree.get(root, x)) >= 0) x = parent;
		return x;
	};
	for (int i = 1; i <= q; ++i) {
		int type, k, a, b;
		std::cin >> type >> k >> a >> b;
		auto root = roots[k + 1];
		a = find(root, a); b = find(root, b);
		if (type == 1) std::cout << (a == b) << '\n';
		else if (a != b) {
			int x = tree.get(root, a), y = tree.get(root, b);
			if (x > y) { std::swap(a, b); std::swap(x, y); }
			root = tree.set(tree.set(root, a, x + y), b, a);
		}
		roots[i] = root;
	}
}
