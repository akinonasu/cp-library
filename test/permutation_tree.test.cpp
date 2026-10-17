#define PROBLEM "https://judge.yosupo.jp/problem/common_interval_decomposition_tree"
#include "../data_structure/permutation_tree.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	std::vector<int> p(n);
	for (int &x : p) in.read(x);
	PermutationTree tree(p);
	std::vector<int> id(tree.size());
	for (int i = 0; i < tree.size(); ++i) id[tree.order()[i]] = i;
	out.println(tree.size());
	for (int v : tree.order()) {
		const auto &node = tree[v];
		out.write(node.parent < 0 ? -1 : id[node.parent]); out.write(' ');
		out.write(node.l); out.write(' ');
		out.write(node.r - 1); out.write(' ');
		out.write(node.type == PermutationTree::Type::Prime ? "prime\n" : "linear\n");
	}
}
