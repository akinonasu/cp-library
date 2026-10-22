#define PROBLEM "https://judge.yosupo.jp/problem/persistent_range_affine_range_sum"
#include "../data_structure/segment_tree/persistent_lazy_segtree.hpp"
#include "../math/mint.hpp"
#include <iostream>
#include <vector>

struct F { mint a, b; };
mint op(mint a, mint b) { return a + b; }
mint e() { return 0; }
mint mapping(F f, mint x, int n) { return f.a * x + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<mint> a(n);
	for (auto &x : a) std::cin >> x;
	PersistentLazySegtree<mint, op, e, F, mapping, composition, id> seg(a);
	std::vector<decltype(seg)::root_type> roots(q + 1, seg.initial_root());
	for (int i = 1; i <= q; ++i) {
		int type, k, l, r;
		std::cin >> type >> k;
		if (type == 0) {
			mint b, c;
			std::cin >> l >> r >> b >> c;
			roots[i] = seg.apply(roots[k + 1], l, r, {b, c});
		} else if (type == 1) {
			int s;
			std::cin >> s >> l >> r;
			roots[i] = seg.copy_range(roots[k + 1], roots[s + 1], l, r);
		} else {
			std::cin >> l >> r;
			std::cout << seg.prod(roots[k + 1], l, r) << '\n';
		}
	}
}
