#define PROBLEM "https://judge.yosupo.jp/problem/double_ended_priority_queue"
#include "../data_structure/top_k_sum.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<int> a(n);
	for (int &x : a) std::cin >> x;
	TopKSum<int> values(a);
	while (q--) {
		int type;
		std::cin >> type;
		if (type == 0) { int x; std::cin >> x; values.insert(x); }
		else {
			auto x = type == 1 ? values.kth_smallest(0) : values.kth_largest(0);
			std::cout << x << '\n';
			values.erase(static_cast<int>(x));
		}
	}
}
