#define PROBLEM "https://judge.yosupo.jp/problem/rooted_tree_isomorphism_classification"
#include "tree/rooted_tree_hash.hpp"
#include "components/fast_io.hpp"


int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<int> parents(n, -1);
	for (int v = 1; v < n; ++v) input.read(parents[v]);
	RootedTreeHash tree;
	tree.build_parents(parents);
	auto answer = tree.classify();
	int count = *std::max_element(answer.begin(), answer.end()) + 1;
	output.write(count);
	output.write('\n');
	for (int v = 0; v < n; ++v) {
		output.write(answer[v]);
		output.write(v + 1 == n ? '\n' : ' ');
	}
}
