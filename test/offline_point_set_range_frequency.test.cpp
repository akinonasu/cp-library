#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_frequency"
#include "data_structure/range_query/offline_range_frequency.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> a(n);
	for (auto &value : a) input.read(value);
	OfflineRangeFrequency queries(a);
	queries.reserve(q);
	while (q--) {
		int type = 0, l = 0, r = 0, x = 0;
		input.read(type), input.read(l);
		if (!type) {
			input.read(x);
			queries.set(l, x);
		} else {
			input.read(r), input.read(x);
			queries.add_query(l, r, x);
		}
	}
	for (int answer : queries.solve()) output.println(answer);
}
