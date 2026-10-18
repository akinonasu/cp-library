#define PROBLEM "https://judge.yosupo.jp/problem/lca"
#include "../graph/auxiliary_tree.hpp"
#include "../graph/lca.hpp"
#include "../components/fast_io.hpp"
#include <array>

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	LCA tree(n);
	for (int v = 1; v < n; ++v) {
		int p = 0;
		in.read(p);
		tree.add_edge(p, v);
	}
	tree.build();
	AuxiliaryTree vt(tree);
	while (q--) {
		std::array<int, 2> keys{};
		in.read(keys[0]), in.read(keys[1]);
		vt.build(keys);
		out.println(vt[vt.root()].vertex);
	}
}
