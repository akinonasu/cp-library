#include "tree/rooted_tree_hash.hpp"
#include "tree/tree_hash.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

int main() {
	std::mt19937_64 random(20261022);
	const int n = 500000;
	std::cout << std::fixed << std::setprecision(6);
	for (const std::string shape : {"path", "star", "binary", "random"}) {
		std::vector<std::vector<int>> graph(n);
		for (int v = 1; v < n; ++v) {
			int p = shape == "path" ? v - 1 : shape == "star" ? 0 : shape == "binary" ? (v - 1) / 2 : int(random() % v);
			graph[p].push_back(v);
			graph[v].push_back(p);
		}
		for (bool unrooted : {false, true}) {
			TreeHashValue expected{};
			for (int run = -1; run < 7; ++run) {
				auto start = std::chrono::steady_clock::now();
				TreeHashValue result;
				if (unrooted) result = tree_hash(graph, 42)[0];
				else {
					RootedTreeHash tree(graph, 0, 42);
					result = tree.get();
				}
				auto stop = std::chrono::steady_clock::now();
				if (run == -1) expected = result;
				else {
					if (result != expected) return 1;
					double ms = std::chrono::duration<double, std::milli>(stop - start).count();
					std::cout << shape << '\t' << n << '\t' << (unrooted ? "unrooted" : "rooted") << '\t' << run << '\t' << ms << '\n';
				}
			}
		}
	}
}
