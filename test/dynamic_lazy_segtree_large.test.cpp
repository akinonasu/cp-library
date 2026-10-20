#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_range_sum_large_array"
#include "../data_structure/segment_tree/dynamic_lazy_segtree.hpp"
#include "../components/fast_io.hpp"
#include "../math/mint.hpp"

struct Sum { mint sum, length; };
struct Affine { mint a, b; };
Sum op(Sum a, Sum b) { return {a.sum + b.sum, a.length + b.length}; }
Sum e() { return {0, 0}; }
Sum mapping(Affine f, Sum x) { return {f.a * x.sum + f.b * x.length, x.length}; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }
Sum initial(std::int64_t l, std::int64_t r) { return {0, r - l}; }

int main() {
	FastInput in;
	FastOutput out;
	std::int64_t n = 0;
	int q = 0;
	in.read(n); in.read(q);
	DynamicLazySegtree<Sum, op, e, Affine, mapping, composition, id, initial> seg(n);
	while (q--) {
		int type = 0;
		std::int64_t l = 0, r = 0;
		in.read(type); in.read(l); in.read(r);
		if (type == 0) {
			unsigned b = 0, c = 0;
			in.read(b); in.read(c);
			seg.apply(l, r, Affine{mint::raw(b), mint::raw(c)});
		} else out.println(seg.prod(l, r).sum.val());
	}
}
