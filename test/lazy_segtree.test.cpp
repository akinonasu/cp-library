#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_range_sum"
#include "data_structure/lazy_segtree.hpp"
#include "components/fast_io.hpp"
#include "math/mint.hpp"

struct Sum { mint sum, length; };
struct Affine { mint a, b; };
Sum op(Sum a, Sum b) { return {a.sum + b.sum, a.length + b.length}; }
Sum e() { return {0, 0}; }
Sum mapping(Affine f, Sum x) { return {f.a * x.sum + f.b * x.length, x.length}; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	std::vector<Sum> a(n);
	for (auto &x : a) { unsigned v = 0; in.read(v); x = {mint::raw(v), 1}; }
	LazySegtree<Sum, op, e, Affine, mapping, composition, id> tree(a);
	while (q--) {
		int type = 0, l = 0, r = 0;
		in.read(type), in.read(l), in.read(r);
		if (type == 0) {
			unsigned b = 0, c = 0;
			in.read(b), in.read(c);
			tree.apply(l, r, Affine{mint::raw(b), mint::raw(c)});
		} else out.println(tree.prod(l, r).sum.val());
	}
}
