#include "tree/prufer_code.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <string>

using Graph = std::vector<std::vector<int>>;

std::vector<int> heap_encode(const Graph &graph) {
	int n = graph.size();
	std::vector<int> degree(n), adjacent(n), code;
	code.reserve(n - 2);
	std::priority_queue<int, std::vector<int>, std::greater<int>> leaves;
	for (int v = 0; v < n; ++v) {
		degree[v] = graph[v].size();
		for (int u : graph[v]) adjacent[v] ^= u;
		if (degree[v] == 1) leaves.push(v);
	}
	for (int i = 0; i < n - 2; ++i) {
		int leaf = leaves.top();
		leaves.pop();
		int parent = adjacent[leaf];
		code.push_back(parent);
		adjacent[parent] ^= leaf;
		if (--degree[parent] == 1) leaves.push(parent);
	}
	return code;
}

Graph heap_decode(const std::vector<int> &code) {
	int n = code.size() + 2;
	Graph graph(n);
	std::vector<int> degree(n, 1);
	for (int v : code) ++degree[v];
	std::priority_queue<int, std::vector<int>, std::greater<int>> leaves;
	for (int v = 0; v < n; ++v) {
		graph[v].reserve(degree[v]);
		if (degree[v] == 1) leaves.push(v);
	}
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

std::uint64_t checksum(const std::vector<int> &values) {
	std::uint64_t result = 0;
	for (int v : values) result = result * 1000000007 + v;
	return result;
}

std::uint64_t checksum(const Graph &graph) {
	std::uint64_t result = 0;
	for (int v = 0; v < int(graph.size()); ++v)
		for (int u : graph[v]) if (v < u) {
			std::uint64_t x = (std::uint64_t(v) << 32) | unsigned(u);
			x ^= x >> 30;
			x *= 0xbf58476d1ce4e5b9ULL;
			result += x ^ (x >> 27);
		}
	return result;
}

template<class F>
double measure(F function, std::uint64_t expected) {
	auto start = std::chrono::steady_clock::now();
	auto result = function();
	auto end = std::chrono::steady_clock::now();
	if (checksum(result) != expected) std::abort();
	return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
	std::mt19937 random(20261021);
	std::cout << "n,shape,operation,heap_ms,linear_ms\n";
	for (int n : {200000, 1000000}) for (std::string shape : {"random", "path", "star"}) {
		std::vector<int> code(n - 2);
		for (int i = 0; i < n - 2; ++i) code[i] = shape == "random" ? random() % n : shape == "path" ? i + 1 : n / 2;
		auto graph = prufer_decode(code);
		const auto code_sum = checksum(code), graph_sum = checksum(graph);
		for (std::string operation : {"encode", "decode"}) {
			std::vector<double> times[2];
			for (int round = 0; round < 7; ++round) {
				int first = random() & 1;
				for (int j = 0; j < 2; ++j) {
					int variant = first ^ j;
					double elapsed;
					if (operation == "encode") elapsed = measure([&] { return variant ? prufer_encode(graph) : heap_encode(graph); }, code_sum);
					else elapsed = measure([&] { return variant ? prufer_decode(code) : heap_decode(code); }, graph_sum);
					times[variant].push_back(elapsed);
				}
			}
			for (auto &values : times) std::sort(values.begin(), values.end());
			std::cout << n << ',' << shape << ',' << operation << ',' << times[0][3] << ',' << times[1][3] << '\n';
		}
	}
}
