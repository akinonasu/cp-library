#define PROBLEM "https://judge.yosupo.jp/problem/bipartitematching"
#include "../graph/matching/bipartite_matching.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int l = 0, r = 0, m = 0;
	in.read(l), in.read(r), in.read(m);
	BipartiteMatching graph(l, r);
	graph.reserve(m);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		in.read(u), in.read(v);
		graph.add_edge(u, v);
	}
	out.println(graph.build());
	for (auto [u, v] : graph.edges()) out.write(u), out.write(' '), out.println(v);
}
