#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_5_D"
#include "tree/edge_link_cut_tree.hpp"
#include "components/fast_io.hpp"

std::int64_t op(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t e() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	EdgeLinkCutTree<std::int64_t, op, e> tree(n);
	std::vector<int> parent(n, -1);
	for (int v = 0; v < n; ++v) {
		int degree = 0;
		input.read(degree);
		while (degree--) {
			int u = 0;
			input.read(u);
			parent[u] = v;
			tree.link(v, u, 0);
		}
	}
	int q = 0;
	input.read(q);
	while (q--) {
		int type = 0, v = 0;
		input.read(type), input.read(v);
		if (type == 0) {
			std::int64_t value = 0;
			input.read(value);
			tree.set(parent[v], v, tree.get(parent[v], v)->forward + value);
		} else output.println(tree.prod(0, v));
	}
}
