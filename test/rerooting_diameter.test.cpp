#define PROBLEM "https://judge.yosupo.jp/problem/tree_diameter"
#include "graph/rerooting.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	using Data = std::pair<long long, int>;
	Rerooting<Data> tree(n);
	std::vector<long long> weights(n - 1);
	std::vector<std::pair<int, int>> edges;
	for (int id = 0; id < n - 1; ++id) {
		int u = 0, v = 0;
		input.read(u), input.read(v), input.read(weights[id]);
		tree.add_edge(u, v), edges.emplace_back(u, v);
	}
	tree.build(Data{-1, -1}, [](Data x, Data y) { return std::max(x, y); },
		[](Data x, int v) { return std::max(x, Data{0, v}); },
		[&](Data x, int, int, int id) { return Data{x.first + weights[id], x.second}; });
	int u = 0;
	for (int v = 1; v < n; ++v) if (tree[v].first > tree[u].first) u = v;
	int target = tree[u].second;
	UndirectedCSR graph(n, edges);
	std::vector<int> previous(n, -1), order{u};
	previous[u] = u;
	for (std::size_t i = 0; i < order.size(); ++i)
		for (auto arc : graph[order[i]]) if (previous[arc.to] == -1) previous[arc.to] = order[i], order.push_back(arc.to);
	std::vector<int> path{target};
	while (path.back() != u) path.push_back(previous[path.back()]);
	output.write(tree[u].first), output.write(' '), output.println(path.size());
	for (int v : path) output.write(v), output.write(' ');
	output.write('\n');
}
