#define PROBLEM "https://judge.yosupo.jp/problem/scc"
#include "graph/scc.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	in.read(n), in.read(m);
	SCCGraph graph(n);
	graph.reserve(m);
	read_unsigned_records<2>(in, m, [&](unsigned from, unsigned to) { graph.add_edge(from, to); });
	auto groups = graph.decomposition();
	out.println(groups.size());
	for (int component = 0; component < groups.size(); ++component) {
		auto group = groups[component];
		out.write(group.size());
		for (int v : group) { out.write(' '); out.write(v); }
		out.write('\n');
	}
}
