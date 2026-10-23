#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite"
#include "../tree/link_cut_tree.hpp"
#include "../math/mint.hpp"
#include <array>
#include "components/fast_io.hpp"
#include <vector>

using namespace std;

using S = array<mint, 2>;
S op(const S &a, const S &b) { return {a[0] * b[0], a[1] * b[0] + b[1]}; }
S e() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	vector<S> values(n);
	for (auto &v : values) {
		unsigned a, b;
		input.read(a), input.read(b);
		v = {a, b};
	}
	LinkCutTree<S, op, e> tree(values);
	std::vector<std::pair<int, int>> edges(n - 1);
	for (auto &[u, v] : edges) input.read(u), input.read(v);
	tree.build_forest(edges);

	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int type, u;
		input.read(type), input.read(u);
		if (type == 0) {
			int v, w, x;
			input.read(v), input.read(w), input.read(x);
			tree.cut(u, v);
			tree.link(w, x);
		} else if (type == 1) {
			unsigned a, b;
			input.read(a), input.read(b);
			S f{a, b};
			tree.set(u, f);
		} else {
			int v;
			unsigned initial;
			input.read(v), input.read(initial);
			mint x = initial;
			auto f = tree.prod(u, v);
			output.println((f[0] * x + f[1]).val());
		}
	}
}
