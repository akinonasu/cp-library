#define PROBLEM "https://judge.yosupo.jp/problem/static_range_count_distinct"
#include "data_structure/range_query/offline_range_count_distinct.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> a(n);
	std::size_t position = 0;
	read_unsigned_records<1>(input, n, [&](std::uint32_t value) { a[position++] = value; });
	OfflineRangeCountDistinct queries(a);
	queries.reserve(q);
	read_unsigned_records<2>(input, q, [&](std::uint32_t l, std::uint32_t r) { queries.add_query(l, r); });
	for (int answer : queries.solve()) output.println(answer);
}
