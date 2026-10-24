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
	struct Item { int value, time; };
	struct Query { int type, rank = 0; };
	std::vector<Item> items(n);
	for (auto &item : items) input.read(item.value), item.time = -1;
	std::vector<Query> queries(q);
	for (int i = 0; i < q; ++i) {
		input.read(queries[i].type);
		if (queries[i].type == 0) {
			int x = 0;
			input.read(x);
			items.push_back({x, i});
		}
	}
	stable_radix_sort<11>(items, [](const Item &item) { return std::uint32_t(item.value) ^ (1U << 31); });
	PredecessorSet values(int(items.size()));
	for (int i = 0; i < int(items.size()); ++i) {
		if (items[i].time < 0) values.insert(i);
		else queries[items[i].time].rank = i;
	}
	for (auto query : queries) {
		if (query.type == 0) values.insert(query.rank);
		else {
			int p = query.type == 1 ? values.min() : values.max();
			output.println(items[p].value);
			values.erase(p);
		}
	}
}
