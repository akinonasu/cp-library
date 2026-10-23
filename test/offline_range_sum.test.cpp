#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum_with_upper_bound"
#include "../data_structure/range_query/offline_range_sum.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>
#include <vector>

using namespace std;

int main() {
	FastInput input;
	FastOutput output;
	uint32_t n = 0, q = 0;
	input.read(n); input.read(q);
	vector<uint32_t> a(n);
	for (auto &value : a) input.read(value);
	OfflineRangeSum<uint32_t, int64_t> queries(a);
	queries.reserve(q);
	for (uint32_t i = 0; i < q; ++i) {
		uint32_t l = 0, r = 0;
		uint32_t x = 0;
		input.read(l); input.read(r); input.read(x);
		queries.add_query(l, r, x + 1);
	}
	for (auto [sum, count] : queries.solve()) {
		output.write(count); output.write(' '); output.println(sum);
	}
}
