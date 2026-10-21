#ifndef PRUFER_CODE_HPP
#define PRUFER_CODE_HPP

#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

inline std::vector<int> prufer_encode(const std::vector<std::vector<int>> &graph) {
	assert(graph.size() <= std::size_t(std::numeric_limits<int>::max()));
	const int n = static_cast<int>(graph.size());
	std::vector<int> degree(n), adjacent(n);
	[[maybe_unused]] std::size_t edges = 0;
	for (int v = 0; v < n; ++v) {
		assert(graph[v].size() < std::size_t(n));
		degree[v] = static_cast<int>(graph[v].size());
		edges += graph[v].size();
		for (int u : graph[v]) {
			assert(0 <= u && u < n && u != v);
			adjacent[v] ^= u;
		}
	}
	assert(edges == (n ? std::size_t(n - 1) * 2 : 0));
	if (n <= 2) return {};
	std::vector<int> code;
	code.reserve(n - 2);
	int next = 0;
	while (next < n && degree[next] != 1) ++next;
	int leaf = next;
	for (int i = 0; i < n - 2; ++i) {
		assert(leaf < n && degree[leaf] == 1);
		const int parent = adjacent[leaf];
		assert(0 <= parent && parent < n && degree[parent] > 0);
		code.push_back(parent);
		degree[leaf] = 0;
		adjacent[parent] ^= leaf;
		if (--degree[parent] == 1 && parent < next) leaf = parent;
		else {
			do { ++next; } while (next < n && degree[next] != 1);
			leaf = next;
		}
	}
	return code;
}

inline std::vector<std::vector<int>> prufer_decode(const std::vector<int> &code, int n) {
	assert(n >= 0 && code.size() == (n >= 2 ? std::size_t(n - 2) : 0));
	std::vector<std::vector<int>> graph(n);
	if (n <= 1) return graph;
	std::vector<int> degree(n, 1);
	for (int v : code) {
		assert(0 <= v && v < n);
		++degree[v];
	}
	for (int v = 0; v < n; ++v) graph[v].reserve(degree[v]);
	int next = 0;
	while (degree[next] != 1) ++next;
	int leaf = next;
	for (int parent : code) {
		graph[leaf].push_back(parent);
		graph[parent].push_back(leaf);
		degree[leaf] = 0;
		if (--degree[parent] == 1 && parent < next) leaf = parent;
		else {
			do { ++next; } while (degree[next] != 1);
			leaf = next;
		}
	}
	graph[leaf].push_back(n - 1);
	graph[n - 1].push_back(leaf);
	return graph;
}

inline std::vector<std::vector<int>> prufer_decode(const std::vector<int> &code) {
	assert(code.size() <= std::size_t(std::numeric_limits<int>::max() - 2));
	return prufer_decode(code, static_cast<int>(code.size()) + 2);
}

#endif
