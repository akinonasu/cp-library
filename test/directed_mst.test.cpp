#define PROBLEM "https://judge.yosupo.jp/problem/directedmst"
#include "graph/directed_mst.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0, root = 0;
	input.read(n), input.read(m), input.read(root);
	DirectedMST<std::int32_t, std::int64_t> graph(n);
	graph.reserve(m);
	for (int i = 0; i < m; ++i) {
		int from = 0, to = 0;
		std::int32_t cost = 0;
		input.read(from), input.read(to), input.read(cost);
		graph.add_edge(from, to, cost);
	}
	auto result = graph.solve(root);
	assert(result);
	output.println(result->cost);
	for (int v = 0; v < n; ++v) {
		if (v) output.write(' ');
		output.write(result->parent[v]);
	}
	output.write('\n');
}
