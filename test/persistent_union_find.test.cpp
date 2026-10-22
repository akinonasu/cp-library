#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#include "../data_structure/persistent_union_find.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	PersistentUnionFind uf(n);
	std::vector<PersistentUnionFind::root_type> roots(q + 1, uf.initial_root());
	for (int i = 1; i <= q; ++i) {
		int type, k, a, b;
		std::cin >> type >> k >> a >> b;
		roots[i] = roots[k + 1];
		if (type == 0) roots[i] = uf.merge(roots[i], a, b);
		else std::cout << uf.same(roots[i], a, b) << '\n';
	}
}
