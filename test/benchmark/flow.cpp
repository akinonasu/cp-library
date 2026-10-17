#include "graph/max_flow.hpp"
#include "components/fast_io.hpp"
#include <string_view>

int main(int argc, char** argv) {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0, s = 0, t = 0;
	input.read(n), input.read(m), input.read(s), input.read(t);
	MaxFlow<> graph(n);
	graph.reserve(m);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		std::int64_t cap = 0;
		input.read(u), input.read(v), input.read(cap);
		graph.add_edge(u - 1, v - 1, cap);
	}
	if (argc > 1 && std::string_view(argv[1]) == "dinic") output.println(graph.flow(s - 1, t - 1, std::numeric_limits<std::int64_t>::max()));
	else output.println(graph.flow(s - 1, t - 1));
}
