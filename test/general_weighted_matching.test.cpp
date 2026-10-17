#define PROBLEM "https://judge.yosupo.jp/problem/general_weighted_matching"
#include "../components/fast_io.hpp"
#include "../graph/general_weighted_matching.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	in.read(n), in.read(m);
	GeneralWeightedMatching graph(n);
	while (m--) {
		int u = 0, v = 0;
		std::int64_t weight = 0;
		in.read(u), in.read(v), in.read(weight);
		graph.add_edge(u, v, weight);
	}
	auto weight = graph.build();
	out.write(graph.matching_size()); out.write(' '); out.println(weight);
	for (auto [u, v] : graph.edges()) { out.write(u); out.write(' '); out.println(v); }
}
