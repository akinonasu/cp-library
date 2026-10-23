#define PROBLEM "https://judge.yosupo.jp/problem/vertex_set_path_composite"
#include "../data_structure/segment_tree/segtree.hpp"
#include "../data_structure/advanced/hld.hpp"
#include "../math/mint.hpp"
#include "components/fast_io.hpp"
#include <vector>

struct S {
	mint a, b, reverse_b;
};
S op(S x, S y) { return {x.a * y.a, x.b * y.a + y.b, y.reverse_b * x.a + x.reverse_b}; }
S e() { return {1, 0, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<S> values(n);
	for (auto &f : values) {
		unsigned a, b;
		input.read(a), input.read(b);
		f.a = a, f.b = b;
		f.reverse_b = f.b;
	}
	HLD tree(n);
	for (int i = 1, u, v; i < n; ++i) {
		input.read(u), input.read(v);
		tree.add_edge(u, v);
	}
	tree.build();
	std::vector<S> ordered(n);
	for (int v = 0; v < n; ++v) ordered[tree.in[v]] = values[v];
	Segtree<S, op, e> seg(ordered);
	while (q--) {
		int type, u, v;
		input.read(type), input.read(u);
		if (type == 0) {
			unsigned a, b;
			input.read(a), input.read(b);
			seg.set(tree.in[u], {a, b, b});
		} else {
			unsigned initial;
			input.read(v), input.read(initial);
			mint x = initial;
			tree.for_each_path(u, v, [&](int l, int r, bool reverse) {
				auto f = seg.prod(l, r);
				x = f.a * x + (reverse ? f.reverse_b : f.b);
			});
			output.println(x.val());
		}
	}
}
