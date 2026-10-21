#include "tree/prufer_code.hpp"
#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <queue>
#include <random>
#include <utility>

using Graph = std::vector<std::vector<int>>;

std::vector<int> reference_encode(const Graph &graph) {
	int n = graph.size();
	if (n <= 2) return {};
	std::vector<int> degree(n), result;
	std::priority_queue<int, std::vector<int>, std::greater<int>> leaves;
	for (int v = 0; v < n; ++v) {
		degree[v] = graph[v].size();
		if (degree[v] == 1) leaves.push(v);
	}
	for (int i = 0; i < n - 2; ++i) {
		int leaf = leaves.top();
		leaves.pop();
		degree[leaf] = 0;
		for (int parent : graph[leaf]) if (degree[parent]) {
			result.push_back(parent);
			if (--degree[parent] == 1) leaves.push(parent);
			break;
		}
	}
	return result;
}

Graph reference_decode(const std::vector<int> &code, int n) {
	Graph graph(n);
	if (n <= 1) return graph;
	std::vector<int> degree(n, 1);
	for (int v : code) ++degree[v];
	std::priority_queue<int, std::vector<int>, std::greater<int>> leaves;
	for (int v = 0; v < n; ++v) if (degree[v] == 1) leaves.push(v);
	for (int parent : code) {
		int leaf = leaves.top();
		leaves.pop();
		graph[leaf].push_back(parent);
		graph[parent].push_back(leaf);
		if (--degree[parent] == 1) leaves.push(parent);
	}
	int a = leaves.top();
	leaves.pop();
	int b = leaves.top();
	graph[a].push_back(b);
	graph[b].push_back(a);
	return graph;
}

Graph normalized(Graph graph) {
	for (auto &row : graph) std::sort(row.begin(), row.end());
	return graph;
}

void check(const std::vector<int> &code, int n) {
	const auto saved = code;
	auto expected = reference_decode(code, n);
	auto actual = prufer_decode(code, n);
	assert(normalized(actual) == normalized(expected));
	assert(prufer_encode(actual) == code);
	assert(prufer_encode(expected) == reference_encode(expected));
	if (n >= 2) assert(prufer_decode(code) == actual);
	assert(code == saved);
}

int main() {
	check({}, 0);
	check({}, 1);
	check({}, 2);
	Graph example{ {1}, {0, 2, 3}, {1}, {1} };
	assert((prufer_encode(example) == std::vector<int>{1, 1}));
	for (int n = 3; n <= 7; ++n) {
		std::vector<int> code(n - 2);
		while (true) {
			check(code, n);
			int i = 0;
			while (i < n - 2 && ++code[i] == n) code[i++] = 0;
			if (i == n - 2) break;
		}
	}
	std::mt19937 random(20261021);
	for (int trial = 0; trial < 2000; ++trial) {
		int n = 3 + random() % 200;
		Graph graph(n);
		std::vector<int> labels(n);
		std::iota(labels.begin(), labels.end(), 0);
		std::shuffle(labels.begin(), labels.end(), random);
		for (int v = 1; v < n; ++v) {
			int a = labels[v], b = labels[random() % v];
			graph[a].push_back(b);
			graph[b].push_back(a);
		}
		for (auto &row : graph) std::shuffle(row.begin(), row.end(), random);
		const auto original = graph;
		auto code = prufer_encode(graph);
		assert(code == reference_encode(graph));
		assert(normalized(prufer_decode(code)) == normalized(graph));
		assert(graph == original);
	}
	for (int center : {0, 100000, 199999}) check(std::vector<int>(199998, center), 200000);
	std::vector<int> path(199998);
	std::iota(path.begin(), path.end(), 1);
	check(path, 200000);
	std::reverse(path.begin(), path.end());
	check(path, 200000);
}
