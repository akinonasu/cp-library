#define PROBLEM "https://judge.yosupo.jp/problem/persistent_range_affine_range_sum"
#include "../data_structure/persistent_lazy_segtree.hpp"
#include "../math/mint.hpp"
#include "../components/fast_io.hpp"
#include <vector>
#include <bit>

struct F { mint a, b; };
mint op(mint a, mint b) { return a + b; }
mint e() { return 0; }
mint mapping(F f, mint x, int n) { return f.a * x + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<mint> a(n);
	for (auto &x : a) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	PersistentLazySegtree<mint, op, e, F, mapping, composition, id> seg(a);
	seg.reserve(2 * a.size() + std::size_t(q) * 3 * std::bit_width(unsigned(n)));
	std::vector<decltype(seg)::root_type> roots(q + 1, seg.initial_root());
	for (int i = 1; i <= q; ++i) {
		int type = 0, k = 0, l = 0, r = 0;
		input.read(type), input.read(k);
		if (type == 0) {
			unsigned b = 0, c = 0;
			input.read(l), input.read(r), input.read(b), input.read(c);
			roots[i] = seg.apply(roots[k + 1], l, r, {mint::raw(b), mint::raw(c)});
		} else if (type == 1) {
			int s = 0;
			input.read(s), input.read(l), input.read(r);
			roots[i] = seg.copy_range(roots[k + 1], roots[s + 1], l, r);
		} else {
			input.read(l), input.read(r);
			output.println(seg.prod(roots[k + 1], l, r).val());
		}
	}
}
