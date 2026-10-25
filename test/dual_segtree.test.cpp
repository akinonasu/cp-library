#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_point_get"
#include "../data_structure/dual_segtree.hpp"
#include "../components/fast_io.hpp"
#include "../math/mint.hpp"

struct Affine { mint a, b; };
mint mapping(Affine f, mint x) { return f.a * x + f.b; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<mint> a(n);
	for (auto &x : a) { unsigned value = 0; in.read(value); x = mint::raw(value); }
	DualSegtree<mint, Affine, mapping, composition, id> seg(std::move(a));
	while (q--) {
		int type = 0, l = 0;
		in.read(type); in.read(l);
		if (type == 0) {
			int r = 0;
			unsigned b = 0, c = 0;
			in.read(r); in.read(b); in.read(c);
			seg.apply(l, r, Affine{mint::raw(b), mint::raw(c)});
		} else out.println(seg.get(l).val());
	}
}
