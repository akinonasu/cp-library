#define PROBLEM "https://judge.yosupo.jp/problem/double_ended_priority_queue"
#include "components/predecessor_set.hpp"
#include "components/radix_sort.hpp"
#include "components/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> initial(n);
	for (int &x : initial) input.read(x);
	struct Item { int value, time; };
	std::vector<Item> items;
	items.reserve(q);
	std::vector<unsigned char> types(q);
	for (auto &type : types) {
		input.read(type);
		if (type == 0) {
			int x = 0;
			input.read(x);
			items.push_back({x, int(items.size())});
		}
	}
	stable_radix_sort<11>(initial, [](int x) { return std::uint32_t(x) ^ (1U << 31); });
	stable_radix_sort<11>(items, [](const Item &item) { return std::uint32_t(item.value) ^ (1U << 31); });
	PredecessorSet values(int(items.size()));
	std::vector<int> ranks(items.size());
	for (int i = 0; i < int(items.size()); ++i) ranks[items[i].time] = i;
	int left = 0, right = n, inserted = 0;
	for (auto type : types) {
		if (type == 0) values.insert(ranks[inserted++]);
		else if (type == 1) {
			int p = values.min();
			if (left < right && (p == values.universe_size() || initial[left] <= items[p].value)) output.println(initial[left++]);
			else { output.println(items[p].value); values.erase(p); }
		} else {
			int p = values.max();
			if (left < right && (p < 0 || initial[right - 1] >= items[p].value)) output.println(initial[--right]);
			else { output.println(items[p].value); values.erase(p); }
		}
	}
}
