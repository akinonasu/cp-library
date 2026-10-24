#define PROBLEM "https://judge.yosupo.jp/problem/static_range_inversions_query"
#include "sequence/static_range_inversions.hpp"
#include "components/fast_io_bulk.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> a(n);
	std::size_t position = 0;
	read_unsigned_records<1>(input, n, [&](std::uint32_t value) { a[position++] = value; });
	std::vector<std::pair<int, int>> ranges(q);
	position = 0;
	read_unsigned_records<2>(input, q, [&](std::uint32_t l, std::uint32_t r) { ranges[position++] = {int(l), int(r)}; });
	auto answer = static_range_inversion_counts(a, ranges);
	for (auto x : answer) output.println(x);
}
