#include "graph/rerooting.hpp"
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

using Data = std::pair<long long, int>;

template<class Merge, class Vertex, class Lift>
std::vector<Data> baseline(int n, const std::vector<std::pair<int, int>> &edges, Merge merge, Vertex vertex, Lift lift) {
	std::vector<std::vector<std::pair<int, int>>> graph(n);
	for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
		auto [u, v] = edges[i];
		graph[u].emplace_back(v, i), graph[v].emplace_back(u, i);
	}
	std::vector<int> parent(n, -1), order{0};
	for (std::size_t i = 0; i < order.size(); ++i) {
		int v = order[i];
		for (auto [to, id] : graph[v]) if (to != parent[v]) parent[to] = v, order.push_back(to);
	}
	std::vector<Data> down(n), up(n), result(n);
	for (auto it = order.rbegin(); it != order.rend(); ++it) {
		int v = *it;
		Data value{0, 0};
		for (auto [to, id] : graph[v]) if (to != parent[v]) value = merge(value, lift(down[to], to, v, id));
		down[v] = vertex(value, v);
	}
	for (int v : order) {
		int d = graph[v].size();
		std::vector<Data> prefix(d + 1), suffix(d + 1), incoming(d);
		for (int i = 0; i < d; ++i) {
			auto [to, id] = graph[v][i];
			incoming[i] = to == parent[v] ? up[v] : lift(down[to], to, v, id);
			prefix[i + 1] = merge(prefix[i], incoming[i]);
		}
		for (int i = d - 1; i >= 0; --i) suffix[i] = merge(incoming[i], suffix[i + 1]);
		result[v] = vertex(prefix[d], v);
		for (int i = 0; i < d; ++i) {
			auto [to, id] = graph[v][i];
			if (to != parent[v]) up[to] = lift(vertex(merge(prefix[i], suffix[i + 1]), v), v, to, id);
		}
	}
	return result;
}

template<class Range>
unsigned long long checksum(const Range &values) {
	unsigned long long h = 1469598103934665603ULL;
	for (auto [sum, count] : values) h = (h ^ static_cast<unsigned long long>(sum)) * 1099511628211ULL + count;
	return h;
}

int main(int argc, char **argv) {
	if (argc != 4) return 1;
	std::string mode = argv[1], algorithm = argv[2];
	int n = std::stoi(argv[3]);
	std::mt19937 rng(731605);
	std::vector<int> labels(n);
	std::iota(labels.begin(), labels.end(), 0);
	std::shuffle(labels.begin(), labels.end(), rng);
	std::vector<std::pair<int, int>> edges;
	for (int v = 1; v < n; ++v) {
		int p = mode == "path" ? v - 1 : mode == "star" ? 0 : mode == "balanced" ? (v - 1) / 2 :
			mode == "broom" ? (v < n / 2 ? v - 1 : n / 2 - 1) : int(rng() % v);
		edges.emplace_back(labels[p], labels[v]);
		if (rng() & 1) std::swap(edges.back().first, edges.back().second);
	}
	std::shuffle(edges.begin(), edges.end(), rng);
	auto merge = [](Data x, Data y) { return Data{x.first + y.first, x.second + y.second}; };
	auto vertex = [](Data x, int) { return Data{x.first, x.second + 1}; };
	auto lift = [](Data x, int, int, int) { return Data{x.first + x.second, x.second}; };
	double elapsed;
	unsigned long long hash;
	if (algorithm == "library") {
		auto start = std::chrono::steady_clock::now();
		Rerooting<Data> tree(n);
		for (auto [u, v] : edges) tree.add_edge(u, v);
		tree.build(Data{0, 0}, merge, vertex, lift);
		auto finish = std::chrono::steady_clock::now();
		elapsed = std::chrono::duration<double, std::milli>(finish - start).count();
		hash = checksum(tree.values());
	} else {
		auto start = std::chrono::steady_clock::now();
		auto result = baseline(n, edges, merge, vertex, lift);
		auto finish = std::chrono::steady_clock::now();
		elapsed = std::chrono::duration<double, std::milli>(finish - start).count();
		hash = checksum(result);
	}
	std::cout << std::fixed << std::setprecision(6) << elapsed << ' ' << hash << '\n';
}
