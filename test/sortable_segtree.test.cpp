#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_sort_range_composite"
#include "../components/fast_io.hpp"
#include "../data_structure/sortable_segtree.hpp"

struct Affine { std::uint32_t a, b; };
Affine compose(Affine f, Affine g) {
	return {std::uint32_t(std::uint64_t(g.a) * f.a % 998244353), std::uint32_t((std::uint64_t(g.a) * f.b + g.b) % 998244353)};
}
Affine identity() { return {1, 0}; }

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	std::vector<std::pair<std::uint32_t, Affine>> a(n);
	for (auto &[key, f] : a) in.read(key), in.read(f.a), in.read(f.b);
	SortableSegtree<std::uint32_t, Affine, compose, identity> tree(a);
	while (q--) {
		int type = 0, l = 0, r = 0;
		in.read(type), in.read(l);
		if (type == 0) {
			std::uint32_t key = 0, x = 0, y = 0;
			in.read(key), in.read(x), in.read(y);
			tree.set(l, key, {x, y});
		} else {
			in.read(r);
			if (type == 1) {
				std::uint32_t x = 0;
				in.read(x);
				auto f = tree.prod(l, r);
				out.println((std::uint64_t(f.a) * x + f.b) % 998244353);
			} else tree.sort(l, r, type == 3);
		}
	}
}
