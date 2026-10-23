#define PROBLEM "https://judge.yosupo.jp/problem/static_range_inversions_query"
#include "data_structure/range_query/mo_order.hpp"
#include "data_structure/fenwick_tree.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> a(n);
	for (auto &x : a) input.read(x);
	auto values = a;
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	for (auto &x : a) x = std::lower_bound(values.begin(), values.end(), x) - values.begin();
	std::vector<std::pair<int, int>> ranges(q);
	for (auto &[l, r] : ranges) input.read(l), input.read(r);
	std::vector<std::int64_t> answer(q);
	FenwickTree<int> counts(values.size());
	int l = 0, r = 0;
	std::int64_t inversions = 0;
	for (int i : mo_order(ranges)) {
		const auto [left, right] = ranges[i];
		while (l > left) { const int x = a[--l]; inversions += counts.sum(x); counts.add(x, 1); }
		while (r < right) { const int x = a[r]; inversions += r - l - counts.sum(x + 1); counts.add(x, 1); ++r; }
		while (l < left) { const int x = a[l++]; counts.add(x, -1); inversions -= counts.sum(x); }
		while (r > right) { const int x = a[--r]; counts.add(x, -1); inversions -= r - l - counts.sum(x + 1); }
		answer[i] = inversions;
	}
	for (auto x : answer) output.println(x);
}
