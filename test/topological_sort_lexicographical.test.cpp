#define PROBLEM "https://atcoder.jp/contests/abc223/tasks/abc223_d"
#include "graph/topological_sort.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	std::vector<std::vector<int>> graph(n);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		input.read(u), input.read(v);
		graph[u - 1].push_back(v - 1);
	}
	auto order = topological_sort<true>(graph);
	if (!order) output.println(-1);
	else for (int i = 0; i < n; ++i) {
		output.write((*order)[i] + 1);
		output.write(i + 1 == n ? '\n' : ' ');
	}
}
