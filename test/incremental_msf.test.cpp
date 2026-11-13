#define PROBLEM "https://judge.yosupo.jp/problem/incremental_minimum_spanning_forest"
#include "graph/incremental_msf.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	IncrementalMSF<int, std::int64_t> graph(n);
	read_unsigned_records<3>(input, m, [&](unsigned u, unsigned v, unsigned cost) {
		output.write(graph.add_edge(int(u), int(v), int(cost)));
		output.write(' ');
	});
	output.write('\n');
}
