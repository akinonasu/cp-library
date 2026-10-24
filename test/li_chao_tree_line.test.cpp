#define PROBLEM "https://judge.yosupo.jp/problem/line_add_get_min"
#include "../data_structure/li_chao_tree/li_chao_tree.hpp"
#include "../components/radix_sort.hpp"
#include <bit>
#include <cstdint>
#include "../components/fast_io.hpp"
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	vector<pair<int64_t, int64_t>> initial(n);
	for (auto &[a, b] : initial) input.read(a), input.read(b);
	struct Query {
		int type, index;
		int64_t a, b;
	};
	vector<Query> queries(q);
	vector<uint64_t> points;
	points.reserve(q);
	for (auto &query : queries) {
		input.read(query.type), input.read(query.a);
		if (query.type == 0) input.read(query.b);
		else points.push_back((uint64_t(uint32_t(query.a) ^ (1U << 31)) << 32) | uint32_t(&query - queries.data()));
	}
	if (points.empty()) return 0;
	stable_radix_sort<11>(points, [](uint64_t point) { return uint32_t(point >> 32); });
	vector<int64_t> xs;
	xs.reserve(points.size());
	for (auto point : points) {
		int64_t value = bit_cast<int32_t>(uint32_t(point >> 32) ^ (1U << 31));
		if (xs.empty() || xs.back() != value) xs.push_back(value);
		queries[uint32_t(point)].index = int(xs.size()) - 1;
	}
	LiChaoTree<int64_t, false, int64_t> tree(std::move(xs));
	for (auto [a, b] : initial) tree.add_line(a, b);
	for (auto query : queries) {
		if (query.type == 0) tree.add_line(query.a, query.b);
		else output.println(*tree.query_index(query.index));
	}
}
