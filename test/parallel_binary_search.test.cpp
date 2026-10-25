#define PROBLEM "https://judge.yosupo.jp/problem/unionfind"
#include "data_structure/range_query/parallel_binary_search.hpp"
#include "data_structure/union_find/union_find.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<std::pair<int, int>> updates, queries;
	std::vector<int> time;
	while (q--) {
		int type = 0, a = 0, b = 0;
		input.read(type), input.read(a), input.read(b);
		if (type == 0) updates.emplace_back(a, b);
		else { queries.emplace_back(a, b); time.push_back(static_cast<int>(updates.size())); }
	}
	UnionFind uf;
	auto first = parallel_binary_search(static_cast<int>(updates.size()), static_cast<int>(queries.size()),
		[&] { uf = UnionFind(n); },
		[&](int id) { auto [a, b] = updates[id]; uf.merge(a, b); },
		[&](int id) { auto [a, b] = queries[id]; return uf.same(a, b); });
	for (std::size_t id = 0; id < queries.size(); ++id) output.println(int(first[id] >= 0 && first[id] <= time[id]));
}
