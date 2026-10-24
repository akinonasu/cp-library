#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#include "../data_structure/range_query/offline_range_kth_smallest.hpp"
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
	vector<array<int, 3>> queries(q);
	std::size_t position = 0;
	read_unsigned_records<3>(input, q, [&](std::uint32_t l, std::uint32_t r, std::uint32_t k) { queries[position++] = {int(l), int(r), int(k)}; });
	for (auto answer : offline_range_kth_smallest(a, queries)) output.println(answer);
}
