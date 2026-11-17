#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#include "../data_structure/weighted_wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<int> initial(n);
	for (auto &x : initial) in.read(x);
	std::vector<int> values = initial;
	std::vector<std::pair<int, int>> queries(q);
	for (auto &[type, x] : queries) {
		in.read(type); in.read(x);
		if (type < 2) values.push_back(x);
	}
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	std::vector<int> weights(values.size());
	for (int x : initial) weights[std::lower_bound(values.begin(), values.end(), x) - values.begin()] = 1;
	WeightedWaveletMatrix<int, int, std::int64_t> wm(values, weights, false);
	int m = int(values.size());
	for (auto [type, x] : queries) {
		if (type < 2) wm.set_weight(int(std::lower_bound(values.begin(), values.end(), x) - values.begin()), type == 0);
		if (type == 2) {
			int count = wm.count(0, m);
			out.println(count < x ? -1 : x <= count / 2 ? wm.kth_smallest(0, m, x - 1) : wm.kth_largest(0, m, count - x));
		}
		if (type == 3) out.println(wm.range_freq(0, m, x) + wm.rank(x, m));
		if (type == 4) out.println(wm.prev_value(0, m, x + 1).value_or(-1));
		if (type == 5) out.println(wm.next_value(0, m, x).value_or(-1));
	}
}
