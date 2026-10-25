#define PROBLEM "https://judge.yosupo.jp/problem/deque"
#include "../data_structure/persistent_deque.hpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	std::cin >> q;
	PersistentDeque<int> deque;
	auto root = deque.initial_root();
	while (q--) {
		int type, x;
		std::cin >> type;
		if (type == 0) { std::cin >> x; root = deque.push_front(root, x); }
		if (type == 1) { std::cin >> x; root = deque.push_back(root, x); }
		if (type == 2) root = deque.pop_front(root);
		if (type == 3) root = deque.pop_back(root);
		if (type == 4) { std::cin >> x; std::cout << deque.get(root, x) << '\n'; }
	}
}
