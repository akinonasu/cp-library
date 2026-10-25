#define PROBLEM "https://judge.yosupo.jp/problem/static_range_frequency"
#include "../data_structure/offline_range_frequency.hpp"
#include "../components/fast_io_bulk.hpp"
#include <cstdint>
#include <vector>

using namespace std;

int main() {
	FastInput input;
	FastOutput output;
	uint32_t n = 0, q = 0;
	input.read(n); input.read(q);
	vector<uint32_t> a(n);
	read_unsigned(input, a);
	OfflineRangeFrequency queries(a);
	queries.reserve(q);
	read_unsigned_records<3>(input, q, [&](std::uint32_t l, std::uint32_t r, std::uint32_t x) { queries.add_query(l, r, x); });
	for (int answer : queries.solve()) output.println(answer);
}
