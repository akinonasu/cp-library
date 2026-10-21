#define PROBLEM "https://judge.yosupo.jp/problem/eulerian_trail_undirected"
#include "graph/eulerian_trail.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int cases = 0;
	input.read(cases);
	while (cases--) {
		int n = 0, m = 0;
		input.read(n), input.read(m);
		UndirectedEulerianTrail graph(n);
		graph.reserve(m);
		for (int i = 0; i < m; ++i) {
			int u = 0, v = 0;
			input.read(u), input.read(v);
			graph.add_edge(u, v);
		}
		auto result = graph.solve();
		if (!result) { output.println("No"); continue; }
		output.println("Yes");
		for (int i = 0; i <= m; ++i) {
			output.write(result->vertices[i]);
			output.write(i == m ? '\n' : ' ');
		}
		for (int i = 0; i < m; ++i) {
			output.write(result->edges[i]);
			output.write(i + 1 == m ? '\n' : ' ');
		}
		if (!m) output.write('\n');
	}
}
