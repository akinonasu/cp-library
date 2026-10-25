#define PROBLEM "https://judge.yosupo.jp/problem/biconnected_components"
#include "graph/block_cut_tree.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	BlockCutTree graph(n);
	graph.edges.reserve(m);
	read_unsigned_records<2>(input, m, [&](unsigned u, unsigned v) { graph.add_edge(u, v); });
	graph.build_components();
	output.println(graph.block_count());
	for (int c = 0; c < graph.block_count(); ++c) {
		auto group = graph.group(c);
		output.write(group.size());
		for (int v : group) output.write(' '), output.write(v);
		output.write('\n');
	}
}
