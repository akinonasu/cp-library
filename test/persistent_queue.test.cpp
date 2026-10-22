#define PROBLEM "https://judge.yosupo.jp/problem/persistent_queue"
#include "../data_structure/persistent_queue.hpp"
#include <iostream>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	std::cin >> q;
	PersistentQueue<int> queue;
	queue.reserve(q);
	std::vector<decltype(queue)::root_type> roots(q + 1);
	for (int i = 1; i <= q; ++i) {
		int type, k;
		std::cin >> type >> k;
		if (type == 0) { int x; std::cin >> x; roots[i] = queue.push(roots[k + 1], x); }
		else { std::cout << queue.front(roots[k + 1]) << '\n'; roots[i] = queue.pop(roots[k + 1]); }
	}
}
