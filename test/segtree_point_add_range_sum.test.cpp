#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"
#include "data_structure/segtree.hpp"
#include "components/fast_io.hpp"
#include <cstdint>
#include <vector>

using Sum = std::int64_t;
Sum op(Sum a, Sum b) { return a + b; }
Sum e() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<Sum> values(n);
	for (auto &value : values) input.read(value);
	Segtree<Sum, op, e> tree(values);
	while (q--) {
		int type = 0, l = 0;
		input.read(type), input.read(l);
		if (type == 0) { Sum value = 0; input.read(value); tree.multiply(l, value); }
		else { int r = 0; input.read(r); output.println(tree.prod(l, r)); }
	}
}
