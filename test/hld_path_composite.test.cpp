#define PROBLEM "https://judge.yosupo.jp/problem/vertex_set_path_composite"
#include "../tree/path_product.hpp"
#include "../data_structure/hld.hpp"
#include "../math/mint.hpp"
#include "components/fast_io_bulk.hpp"
#include <vector>

struct S {
	mint a, b, reverse_b;
};
inline S op(S x, S y) { return {x.a * y.a, x.b * y.a + y.b, y.reverse_b * x.a + x.reverse_b}; }
inline S e() { return {1, 0, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<S> values(n);
	std::size_t position = 0;
	read_unsigned_records<2>(input, n, [&](unsigned a, unsigned b) { values[position++] = {a, b, b}; });
	HLD tree(n);
	read_unsigned_records<2>(input, n - 1, [&](unsigned u, unsigned v) { tree.add_edge(u, v); });
	tree.build();
	TreePathProduct<S, op, e, [](S value) {
		std::swap(value.b, value.reverse_b);
		return value;
	}> seg(tree, values);
	read_unsigned_records<4>(input, q, [&](unsigned type, unsigned u, unsigned a, unsigned b) {
		if (type == 0) seg.set(u, {a, b, b});
		else {
			auto x = seg.apply(u, a, mint(b), [](const S& f, mint value) { return f.a * value + f.b; });
			output.println(x.val());
		}
	});
}
