#define PROBLEM "https://judge.yosupo.jp/problem/minimum_steiner_tree"
#include "../graph/steiner_tree.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	SteinerTree<> graph(n);
	graph.reserve(m);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		std::int64_t w = 0;
		input.read(u), input.read(v), input.read(w);
		graph.add_edge(u, v, w);
	}
	int k = 0;
	input.read(k);
	std::vector<int> terminals(k);
	for (int &v : terminals) input.read(v);
	auto result = graph.solve(terminals);
	assert(result);
	output.write(result->cost), output.write(' '), output.write(result->edges.size()), output.write('\n');
	for (std::size_t i = 0; i < result->edges.size(); ++i) {
		if (i) output.write(' ');
		output.write(result->edges[i]);
	}
	output.write('\n');
}
