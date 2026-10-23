#define PROBLEM "https://judge.yosupo.jp/problem/jump_on_tree"
#include "graph/functional_graph.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n);
	input.read(q);
	std::vector<std::vector<int>> graph(n);
	for (int i = 1; i < n; ++i) {
		int u = 0, v = 0;
		input.read(u);
		input.read(v);
		graph[u].push_back(v);
		graph[v].push_back(u);
	}
	std::vector<int> next(n, -1), order{0};
	next[0] = 0;
	for (std::size_t i = 0; i < order.size(); ++i) {
		int v = order[i];
		for (int u : graph[v]) if (u != next[v]) next[u] = v, order.push_back(u);
	}
	FunctionalGraph tree(std::move(next));
	while (q--) {
		int u = 0, v = 0, k = 0;
		input.read(u);
		input.read(v);
		input.read(k);
		int w = tree.lca(u, v), a = tree.depth[u] - tree.depth[w], b = tree.depth[v] - tree.depth[w];
		int answer = k > a + b ? -1 : k <= a ? tree.jump(u, k) : tree.jump(v, a + b - k);
		output.write(answer);
		output.write('\n');
	}
}
