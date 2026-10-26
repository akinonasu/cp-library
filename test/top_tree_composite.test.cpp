#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite"
#include "tree/top_tree.hpp"
#include "components/fast_io.hpp"
#include "test/fixtures/top_tree_dp.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<TopTreeCompositeDP::Vertex> values(n);
	for (auto& v : values) {
		unsigned a, b;
		input.read(a), input.read(b);
		v = {a, b};
	}
	TopTree<TopTreeCompositeDP> tree(std::move(values));
	for (int i = 1; i < n; ++i) {
		int u, v;
		input.read(u), input.read(v);
		tree.link(u, v);
	}
	while (q--) {
		int type, u;
		input.read(type), input.read(u);
		if (type == 0) {
			int v, w, x;
			input.read(v), input.read(w), input.read(x);
			tree.cut(u, v), tree.link(w, x);
		} else if (type == 1) {
			unsigned a, b;
			input.read(a), input.read(b);
			tree.set(u, {a, b});
		} else {
			int v;
			unsigned x;
			input.read(v), input.read(x);
			auto f = tree.prod(u, v);
			output.println((f[0] * x + f[1]).val());
		}
	}
}
