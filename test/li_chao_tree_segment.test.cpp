#define PROBLEM "https://judge.yosupo.jp/problem/segment_add_get_min"
#include "../data_structure/li_chao_tree/li_chao_tree.hpp"
#include "../components/radix_sort.hpp"
#include <bit>
#include <algorithm>
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
	struct Segment {
		int64_t l, r, a, b;
	};
	vector<Segment> initial(n);
	for (auto &[l, r, a, b] : initial) input.read(l), input.read(r), input.read(a), input.read(b);
	struct Query {
		int type, index;
		Segment f;
	};
	vector<Query> queries(q);
	vector<uint64_t> points;
	points.reserve(2 * initial.size() + 2 * queries.size());
	auto endpoint = [&](int64_t value, unsigned tag) { points.push_back((uint64_t(uint32_t(value) ^ (1U << 31)) << 32) | tag); };
	for (int i = 0; i < n; ++i) { endpoint(initial[i].l, 4U * i); endpoint(initial[i].r, 4U * i + 1); }
	for (auto &query : queries) {
		input.read(query.type), input.read(query.f.l);
		unsigned tag = 4U * unsigned(n + (&query - queries.data()));
		if (query.type == 0) {
			input.read(query.f.r), input.read(query.f.a), input.read(query.f.b);
			endpoint(query.f.l, tag), endpoint(query.f.r, tag + 1);
		} else endpoint(query.f.l, tag + 2);
	}
	if (none_of(queries.begin(), queries.end(), [](const Query &query) { return query.type == 1; })) return 0;
	stable_radix_sort<11>(points, [](uint64_t point) { return uint32_t(point >> 32); });
	vector<int64_t> xs;
	xs.reserve(q);
	for (size_t first = 0; first < points.size();) {
		size_t last = first + 1;
		while (last < points.size() && uint32_t(points[last] >> 32) == uint32_t(points[first] >> 32)) ++last;
		bool queried = false;
		for (size_t i = first; i < last; ++i) {
			unsigned tag = uint32_t(points[i]), owner = tag / 4;
			if ((tag & 3) == 2) { queries[owner - n].index = int(xs.size()); queried = true; }
			else {
				auto &segment = owner < unsigned(n) ? initial[owner] : queries[owner - n].f;
				(tag & 1 ? segment.r : segment.l) = int(xs.size());
			}
		}
		if (queried) xs.push_back(bit_cast<int32_t>(uint32_t(points[first] >> 32) ^ (1U << 31)));
		first = last;
	}
	LiChaoTree<int64_t, false, int64_t> tree(std::move(xs));
	for (auto [l, r, a, b] : initial) tree.add_segment_index(int(l), int(r), a, b);
	for (auto query : queries) {
		if (query.type == 0) tree.add_segment_index(int(query.f.l), int(query.f.r), query.f.a, query.f.b);
		else {
			auto value = tree.query_index(query.index);
			if (value) output.println(*value);
			else output.write("INFINITY\n");
		}
	}
}
