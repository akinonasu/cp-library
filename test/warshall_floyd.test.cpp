#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_1_C"
#include "graph/warshall_floyd.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n); input.read(m);
	WarshallFloyd<> graph(n);
	for (int i = 0; i < m; ++i) {
		int from = 0, to = 0;
		std::int64_t weight = 0;
		input.read(from); input.read(to); input.read(weight);
		graph.add_edge(from, to, weight);
	}
	if (!graph.solve()) { output.println("NEGATIVE CYCLE"); return 0; }
	for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
		if (j) output.write(' ');
		if (graph(i, j) == graph.infinity()) output.write("INF");
		else output.write(graph(i, j));
		if (j + 1 == n) output.write('\n');
	}
}
