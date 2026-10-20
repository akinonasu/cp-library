#ifndef BIPARTITE_MATCHING_HEADER
#define BIPARTITE_MATCHING_HEADER "graph/matching/bipartite_matching.hpp"
#endif
#include BIPARTITE_MATCHING_HEADER
#include ".verify-helper/cache/bipartite_matching/baseline.hpp"
#include ".verify-helper/cache/bipartite_matching/reference_core.hpp"
#include "components/fast_io.hpp"
#include <chrono>
#include <cstdio>
#include <string>

int main(int argc, char** argv) {
	FastInput in;
	int l = 0, r = 0, m = 0;
	in.read(l), in.read(r), in.read(m);
	std::vector<std::pair<int, int>> input(m);
	for (auto& [u, v] : input) in.read(u), in.read(v);
	auto begin = std::chrono::steady_clock::now();
	int size;
	if (argc > 1 && std::string(argv[1]) == "reference") {
		for (auto [u, v] : input) ++deg[u], ++deg[l + v];
		for (int i = 0; i <= l + r; ++i) deg[i + 1] += deg[i];
		for (auto [u, v] : input) e[--deg[u]] = l + v, e[--deg[l + v]] = u;
		size = HopcroftKarp::run(l, l + r);
	} else if (argc > 1 && std::string(argv[1]) == "baseline") {
		BaselineBipartiteMatching graph(l, r);
		graph.reserve(m);
		for (auto [u, v] : input) graph.add_edge(u, v);
		size = graph.build();
	} else {
		BipartiteMatching graph(l, r);
		graph.reserve(m);
		for (auto [u, v] : input) graph.add_edge(u, v);
		size = graph.build();
	}
	auto end = std::chrono::steady_clock::now();
	std::printf("%d %.6f\n", size, std::chrono::duration<double, std::milli>(end - begin).count());
}
