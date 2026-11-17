#define PROBLEM "https://judge.yosupo.jp/problem/jump_on_tree"
#include "tree/edge_link_cut_tree.hpp"
#include "components/fast_io.hpp"

int op(int a, int b) { return a + b; }
int e() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	EdgeLinkCutTree<int, op, e> tree(n);
	for (int i = 1; i < n; ++i) {
		int u = 0, v = 0;
		input.read(u), input.read(v);
		tree.link(u, v, 0);
	}
	while (q--) {
		int u = 0, v = 0, k = 0;
		input.read(u), input.read(v), input.read(k);
		output.println(tree.jump(u, v, k));
	}
}
