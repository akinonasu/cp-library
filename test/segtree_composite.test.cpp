#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"
#include "data_structure/segment_tree/segtree.hpp"
#include "components/fast_io.hpp"
#include "math/mint.hpp"
#include <utility>
#include <vector>

using S = std::pair<mint, mint>;
S op(S f, S g) { return {g.first * f.first, g.first * f.second + g.second}; }
S e() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<S> values(n);
	for (auto &[a, b] : values) {
		unsigned x = 0, y = 0;
		input.read(x), input.read(y);
		a = mint::raw(x), b = mint::raw(y);
	}
	Segtree<S, op, e> tree(values);
	while (q--) {
		int type = 0, l = 0;
		input.read(type), input.read(l);
		if (type == 0) {
			unsigned a = 0, b = 0;
			input.read(a), input.read(b);
			tree.set(l, {mint::raw(a), mint::raw(b)});
		} else {
			int r = 0;
			unsigned x = 0;
			input.read(r), input.read(x);
			auto [a, b] = tree.prod(l, r);
			output.println((a * mint::raw(x) + b).val());
		}
	}
}
