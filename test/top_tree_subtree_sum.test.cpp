#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_subtree_sum"
#include "tree/top_tree.hpp"
#include "components/fast_io.hpp"
#include "test/fixtures/top_tree_dp.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<std::int64_t> values(n);
	for (auto& v : values) input.read(v);
	TopTree<TopTreeSumDP> tree(std::move(values));
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
			std::int64_t x;
			input.read(x);
			tree.set(u, tree.get(u) + x);
		} else {
			int p;
			input.read(p);
			output.println(tree.subtree_prod(p, u));
		}
	}
}
