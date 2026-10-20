#include "components/mo_order.hpp"
#include "components/undirected_csr.hpp"
#include "graph/connectivity/low_link.hpp"
#include "graph/connectivity/two_edge_connected_components.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

int main(int argc, char **argv) {
	if (argc != 6) return 1;
	std::string mode = argv[1], shape = argv[2];
	int n = std::stoi(argv[3]), m = std::stoi(argv[4]), rounds = std::stoi(argv[5]);
	std::mt19937 rng(20261020);
	std::vector<std::pair<int, int>> input;
	for (int i = 0; i < m; ++i) {
		int u = rng() % n, v = rng() % n;
		if (shape == "path") u = i % n, v = (u + 1) % n;
		if (shape == "star") u = 0;
		if (shape == "duplicates") u %= 16, v %= 16;
		if (mode == "mo" && u > v) std::swap(u, v);
		input.emplace_back(u, v);
	}
	if (shape == "sorted") std::sort(input.begin(), input.end());
	double elapsed = 0;
	std::uint64_t checksum = 0;
	for (int trial = 0; trial < rounds; ++trial) {
		auto begin = std::chrono::steady_clock::now();
		auto stop = [&] { elapsed += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count(); };
		if (mode == "mo") {
			auto order = mo_order(input);
			stop();
			for (int x : order) checksum = checksum * 1000003 + x;
		} else if (mode == "csr") {
			UndirectedCSR graph(n, input);
			stop();
			for (int v = 0; v < n; ++v) for (auto arc : graph[v]) checksum = checksum * 1000003 + arc.to + arc.id;
		} else if (mode == "lowlink") {
			LowLink graph(n);
			graph.edges = input;
			graph.build();
			stop();
			for (int x : graph.low) checksum = checksum * 1000003 + x;
			for (int x : graph.bridges) checksum = checksum * 1000003 + x;
		} else if (mode == "twoedge") {
			TwoEdgeConnectedComponents graph(n);
			graph.edges = input;
			graph.build();
			stop();
			for (int x : graph.component) checksum = checksum * 1000003 + x;
		} else return 1;
	}
	std::cout << std::fixed << std::setprecision(6) << elapsed << ' ' << checksum << '\n';
}
