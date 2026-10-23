#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_path_sum"
#include "../tree/link_cut_tree.hpp"
#include <cstdint>
#include "components/fast_io.hpp"
#include <vector>

using namespace std;

int64_t op(int64_t a, int64_t b) { return a + b; }
int64_t e() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	vector<int64_t> values(n);
	for (auto &v : values) input.read(v);
	LinkCutTree<int64_t, op, e, true> tree(values);
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
			int64_t x;
			input.read(x);
			tree.set(u, tree.get(u) + x);
		} else {
			int v;
			input.read(v);
			output.println(tree.prod(u, v));
		}
	}
}
