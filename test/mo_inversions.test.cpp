#define PROBLEM "https://judge.yosupo.jp/problem/static_range_inversions_query"
#include "sequence/static_range_inversions.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> a(n);
	for (auto &x : a) input.read(x);
	std::vector<std::pair<int, int>> ranges(q);
	for (auto &[l, r] : ranges) input.read(l), input.read(r);
	auto answer = static_range_inversion_counts(a, ranges);
	for (auto x : answer) output.println(x);
}
