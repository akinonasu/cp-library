#define PROBLEM "https://judge.yosupo.jp/problem/scc"
#include "graph/connectivity/scc.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	in.read(n), in.read(m);
	SCCGraph graph(n);
	graph.reserve(m);
	while (m--) {
		int from = 0, to = 0;
		in.read(from), in.read(to);
		graph.add_edge(from, to);
	}
	auto groups = graph.scc();
	out.println(groups.size());
	for (const auto &group : groups) {
		out.write(group.size());
		for (int v : group) { out.write(' '); out.write(v); }
		out.write('\n');
	}
}
