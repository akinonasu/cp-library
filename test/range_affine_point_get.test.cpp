#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_point_get"
#include "data_structure/segment_tree/range_affine_range_sum.hpp"
#include "components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<mint> values(n);
	for (auto &value : values) { unsigned x = 0; input.read(x); value = mint::raw(x); }
	RangeAffineRangeSum tree(values);
	while (q--) {
		int type = 0, l = 0;
		input.read(type), input.read(l);
		if (type == 0) {
			int r = 0;
			unsigned a = 0, b = 0;
			input.read(r), input.read(a), input.read(b);
			tree.apply(l, r, mint::raw(a), mint::raw(b));
		} else output.println(tree.get(l).val());
	}
}
