#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_range_sum"
#include "data_structure/segment_tree/range_affine_range_sum.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<mint> a(n);
	for (auto &x : a) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	RangeAffineRangeSum tree(a);
	while (q--) {
		int type = 0, l = 0, r = 0;
		input.read(type), input.read(l), input.read(r);
		if (!type) {
			unsigned b = 0, c = 0;
			input.read(b), input.read(c);
			tree.apply(l, r, mint::raw(b), mint::raw(c));
		} else output.println(tree.prod(l, r).val());
	}
}
