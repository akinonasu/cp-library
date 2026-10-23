#define PROBLEM "https://judge.yosupo.jp/problem/range_add_range_min"
#include "data_structure/segment_tree/range_add_range_min.hpp"
#include "components/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<std::int64_t> values(n);
	for (auto &value : values) input.read(value);
	RangeAddRangeMin tree(values);
	while (q--) {
		int type = 0, l = 0, r = 0;
		input.read(type), input.read(l), input.read(r);
		if (type == 0) { std::int64_t value = 0; input.read(value); tree.add(l, r, value); }
		else output.println(tree.prod(l, r));
	}
}
