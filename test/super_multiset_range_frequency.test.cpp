#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_frequency"
#include "data_structure/super_multiset.hpp"
#include "components/fast_io.hpp"
#include <unordered_map>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> values(n);
	std::unordered_map<int, SmallestKSum<int>> positions;
	positions.reserve(n + q);
	for (int i = 0; i < n; ++i) {
		input.read(values[i]);
		positions[values[i]].insert(i);
	}
	while (q--) {
		int type = 0, l = 0, r = 0, x = 0;
		input.read(type), input.read(l);
		if (type == 0) {
			input.read(x);
			positions[values[l]].erase(l);
			positions[x].insert(l);
			values[l] = x;
		} else {
			input.read(r), input.read(x);
			auto it = positions.find(x);
			output.println(it == positions.end() ? 0 : it->second.range_freq(l, r));
		}
	}
}
