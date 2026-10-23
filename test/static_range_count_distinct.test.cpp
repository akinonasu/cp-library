#define PROBLEM "https://judge.yosupo.jp/problem/static_range_count_distinct"
#include "data_structure/range_query/offline_range_count_distinct.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> a(n);
	for (auto &value : a) input.read(value);
	OfflineRangeCountDistinct queries(a);
	queries.reserve(q);
	while (q--) {
		int l = 0, r = 0;
		input.read(l), input.read(r);
		queries.add_query(l, r);
	}
	for (int answer : queries.solve()) output.println(answer);
}
