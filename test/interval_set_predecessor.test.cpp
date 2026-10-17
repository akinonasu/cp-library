#define PROBLEM "https://judge.yosupo.jp/problem/predecessor_problem"
#include "data_structure/interval_set.hpp"
#include "components/fast_io.hpp"
#include <string_view>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0; input.read(n); input.read(q);
	std::string_view initial; input.read(initial);
	std::vector<std::pair<int, int>> intervals;
	for (int i = 0; i < n;) {
		if (initial[i] == '0') { ++i; continue; }
		int l = i;
		while (i < n && initial[i] == '1') ++i;
		intervals.emplace_back(l, i);
	}
	IntervalSet<int> set(std::move(intervals));
	while (q--) {
		int t = 0, k = 0; input.read(t); input.read(k);
		if (t == 0) set.insert(k);
		else if (t == 1) set.erase(k);
		else if (t == 2) output.println(static_cast<int>(set.contains(k)));
		else if (t == 3) output.println(set.next(k).value_or(-1));
		else output.println(set.prev(k).value_or(-1));
	}
}
