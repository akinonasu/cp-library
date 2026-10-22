#include "graph/functional_graph.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

int main() {
	const int n = 500000, q = 500000;
	std::mt19937_64 random(20261022);
	std::cout << std::fixed << std::setprecision(6);
	for (const std::string shape : {"chain", "cycle", "tail_cycle", "self_loops", "binary", "random"}) {
		std::vector<int> next(n);
		for (int v = 0; v < n; ++v) {
			if (shape == "chain") next[v] = std::min(v + 1, n - 1);
			else if (shape == "cycle") next[v] = (v + 1) % n;
			else if (shape == "tail_cycle") next[v] = v + 1 == n ? n / 2 : v + 1;
			else if (shape == "self_loops") next[v] = v;
			else if (shape == "binary") next[v] = v ? (v - 1) / 2 : 0;
			else next[v] = random() % n;
		}
		FunctionalGraph graph(next);
		struct Query { int u, v; std::uint64_t k; };
		std::vector<Query> queries(q);
		for (int i = 0; i < q; ++i) {
			int v = random() % n;
			queries[i] = {int(random() % n), v, i & 1 ? random() : random() % (std::uint64_t(graph.depth[v]) + 1)};
		}
		for (const std::string operation : {"build", "jump", "dist"}) {
			std::uint64_t expected = 0;
			for (int run = -1; run < 7; ++run) {
				std::uint64_t checksum = 0;
				auto start = std::chrono::steady_clock::now();
				if (operation == "build") {
					FunctionalGraph temporary(next);
					checksum = temporary.cycle_count() + temporary.jump(n / 2, std::numeric_limits<std::uint64_t>::max());
				} else if (operation == "jump") {
					for (auto query : queries) checksum = checksum * 1000000007 + graph.jump(query.v, query.k);
				} else {
					for (auto query : queries) checksum = checksum * 1000000007 + std::uint64_t(graph.dist(query.u, query.v));
				}
				auto stop = std::chrono::steady_clock::now();
				if (run == -1) expected = checksum;
				else {
					if (checksum != expected) return 1;
					double ms = std::chrono::duration<double, std::milli>(stop - start).count();
					std::cout << shape << '\t' << operation << '\t' << run << '\t' << ms << '\n';
				}
			}
		}
	}
}
