#define PROBLEM "https://judge.yosupo.jp/problem/vertex_set_path_composite"
#include "tree/path_product.hpp"
#include "math/mint.hpp"
#include "components/fast_io.hpp"

using S = std::pair<mint, mint>;
S op(S a, S b) { return {a.first * b.first, a.second * b.first + b.second}; }
S e() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<S> values(n);
	for (auto& [a, b] : values) {
		unsigned x = 0, y = 0;
		input.read(x), input.read(y);
		a = x, b = y;
	}
	HLD hld(n);
	for (int i = 1; i < n; ++i) {
		int u = 0, v = 0;
		input.read(u), input.read(v);
		hld.add_edge(u, v);
	}
	hld.build();
	TreePathProduct<S, op, e> tree(hld, values);
	while (q--) {
		int type = 0, u = 0;
		input.read(type), input.read(u);
		if (type == 0) {
			unsigned a = 0, b = 0;
			input.read(a), input.read(b);
			tree.set(u, {a, b});
		} else {
			int v = 0;
			unsigned x = 0;
			input.read(v), input.read(x);
			auto [a, b] = tree.prod(u, v);
			output.println((a * mint(x) + b).val());
		}
	}
}
