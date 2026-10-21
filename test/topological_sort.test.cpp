#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_4_A"
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
		graph[u].push_back(v);
	}
	output.println(!topological_sort(graph));
}
