#define PROBLEM "https://judge.yosupo.jp/problem/rooted_tree_isomorphism_classification"
#include "tree/rooted_tree_hash.hpp"
#include "components/fast_io.hpp"
#include <unordered_map>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<std::vector<int>> graph(n);
	for (int v = 1; v < n; ++v) {
		int p = 0;
		input.read(p);
		graph[p].push_back(v);
		graph[v].push_back(p);
	}
	RootedTreeHash tree(graph);
	std::unordered_map<TreeHashValue, int, TreeHashValue::Hasher> ids;
	ids.reserve(n);
	std::vector<int> answer(n);
	for (int v = 0; v < n; ++v) answer[v] = ids.try_emplace(tree[v], static_cast<int>(ids.size())).first->second;
	output.write(static_cast<int>(ids.size()));
	output.write('\n');
	for (int v = 0; v < n; ++v) {
		output.write(answer[v]);
		output.write(v + 1 == n ? '\n' : ' ');
	}
}
