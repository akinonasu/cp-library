#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum"
#include "../data_structure/lazy_reversible_treap.hpp"
#include "../components/fast_io.hpp"
#include "../math/mint.hpp"
#include <vector>

struct F { mint a, b; };
mint op(mint a, mint b) { return a + b; }
mint e() { return 0; }
mint mapping(F f, mint s, int n) { return f.a * s + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	std::vector<mint> a(n);
	for (auto &x : a) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	LazyReversibleTreap<mint, op, e, F, mapping, composition, id, true> tree(a);
	tree.reserve(n + q);
	while (q--) {
		int type = 0, l = 0;
		input.read(type); input.read(l);
		if (type == 0) { unsigned x = 0; input.read(x); tree.insert(l, mint::raw(x)); }
		else if (type == 1) tree.erase(l);
		else {
			int r = 0;
			input.read(r);
			if (type == 2) tree.reverse(l, r);
			else if (type == 3) { unsigned b = 0, c = 0; input.read(b); input.read(c); tree.apply(l, r, {mint::raw(b), mint::raw(c)}); }
			else { output.write(tree.prod(l, r).val()); output.write('\n'); }
		}
	}
}
