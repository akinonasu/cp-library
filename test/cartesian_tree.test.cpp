#define PROBLEM "https://judge.yosupo.jp/problem/cartesian_tree"
#include "tree/cartesian_tree.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<int> values(n);
	for (int &value : values) input.read(value);
	auto tree = cartesian_tree(values);
	for (int v = 0; v < n; ++v) {
		output.write(tree.parent[v] == -1 ? v : tree.parent[v]);
		output.write(v + 1 == n ? '\n' : ' ');
	}
}
