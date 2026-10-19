#include "graph/connectivity/two_edge_connected_components.hpp"
#include "components/fast_io.hpp"
#include "two_edge_connected_components_baseline.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
#if TWO_EDGE_BASELINE
	LowLinkComponents graph(n);
#else
	TwoEdgeConnectedComponents graph(n);
#endif
	graph.edges.reserve(m);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		input.read(u), input.read(v), graph.add_edge(u, v);
	}
	graph.build();
	output.println(graph.component_count());
	for (int c = 0; c < graph.component_count(); ++c) {
		auto group = graph.group(c);
		output.write(group.size());
		for (int v : group) output.write(' '), output.write(v);
		output.write('\n');
	}
}
