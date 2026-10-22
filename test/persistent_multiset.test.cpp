#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#include "../data_structure/persistent_multiset.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	PersistentMultiset<int> set;
	std::vector<int> a(n);
	for (int &x : a) std::cin >> x;
	auto root = set.build(a);
	while (q--) {
		int type, x;
		std::cin >> type >> x;
		if (type == 0) { if (!set.contains(root, x)) root = set.insert(root, x); }
		if (type == 1) root = set.erase_all(root, x);
		if (type == 2) std::cout << (set.size(root) < x ? -1 : set.kth(root, x - 1)) << '\n';
		if (type == 3) std::cout << set.order_of_key(root, x) + set.count(root, x) << '\n';
		if (type == 4) { auto k = set.order_of_key(root, x) + set.count(root, x); std::cout << (k ? set.kth(root, k - 1) : -1) << '\n'; }
		if (type == 5) std::cout << set.lower_bound(root, x).value_or(-1) << '\n';
	}
}
