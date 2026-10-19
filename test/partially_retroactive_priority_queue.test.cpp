#define PROBLEM "https://judge.yosupo.jp/problem/double_ended_priority_queue"
#include "../data_structure/priority_queue/partially_retroactive_priority_queue.hpp"
#include <cassert>
#include <functional>
#include <iostream>
#include <utility>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	using Key = std::pair<int, int>;
	PartiallyRetroactivePriorityQueue<Key> minimum;
	PartiallyRetroactivePriorityQueue<Key, std::greater<Key>> maximum;
	minimum.reserve(n + q); maximum.reserve(n + q);
	for (int time = 0; time < n + q; ++time) {
		int command = 0;
		if (time >= n) std::cin >> command;
		if (command == 0) {
			int value;
			std::cin >> value;
			bool a = minimum.insert(time, {value, time}), b = maximum.insert(time, {value, time});
			assert(a && b); (void)a; (void)b;
		} else if (command == 1) {
			auto [value, insertion] = *minimum.top_ptr();
			std::cout << value << '\n';
			bool a = minimum.insert_pop(time), b = maximum.erase(insertion);
			assert(a && b); (void)a; (void)b;
		} else {
			auto [value, insertion] = *maximum.top_ptr();
			std::cout << value << '\n';
			bool a = maximum.insert_pop(time), b = minimum.erase(insertion);
			assert(a && b); (void)a; (void)b;
		}
	}
}
