#define PROBLEM "https://judge.yosupo.jp/problem/persistent_queue"
#include "../data_structure/persistent_heap.hpp"
#include <iostream>
#include <utility>
#include <vector>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	std::cin >> q;
	PersistentHeap<std::pair<int, int>> heap;
	std::vector<decltype(heap)::root_type> roots(q + 1);
	for (int i = 1; i <= q; ++i) {
		int type, k;
		std::cin >> type >> k;
		if (type == 0) { int x; std::cin >> x; roots[i] = heap.push(roots[k + 1], {i, x}); }
		else { std::cout << heap.top(roots[k + 1]).second << '\n'; roots[i] = heap.pop(roots[k + 1]); }
	}
}
