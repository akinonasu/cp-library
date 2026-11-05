#include "tree/tree_center.hpp"
#include "tree/prufer_code.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>

using Graph = std::vector<std::vector<int>>;

std::vector<int> reference(const Graph &graph) {
	const int n = static_cast<int>(graph.size());
	std::vector<int> centers;
	int radius = n;
	for (int source = 0; source < n; ++source) {
		std::vector<int> distance(n, -1), queue{source};
		distance[source] = 0;
		int eccentricity = 0;
		for (std::size_t i = 0; i < queue.size(); ++i) for (int u : graph[queue[i]]) if (distance[u] == -1) {
			distance[u] = distance[queue[i]] + 1;
			eccentricity = std::max(eccentricity, distance[u]);
			queue.push_back(u);
		}
		if (eccentricity < radius) radius = eccentricity, centers.clear();
		if (eccentricity == radius) centers.push_back(source);
	}
	return centers;
}

void check(const Graph &graph) {
	const auto saved = graph;
	const auto result = tree_center(graph);
	assert(result == reference(graph));
	assert(graph == saved && tree_center(graph) == result);
}

int main() {
	check({}); check(Graph(1)); check({ {1}, {0} });
	for (int n = 2; n <= 6; ++n) {
		std::vector<int> code(n - 2);
		bool done = false;
		while (!done) {
			check(prufer_decode(code, n));
			int i = 0;
			while (i < n - 2 && ++code[i] == n) code[i++] = 0;
			done = i == n - 2;
		}
	}
	std::mt19937 random(902731);
	for (int n = 1; n <= 100; ++n) for (int repeat = 0; repeat < 15; ++repeat) {
		std::vector<int> label(n); std::iota(label.begin(), label.end(), 0); std::shuffle(label.begin(), label.end(), random);
		Graph graph(n);
		for (int v = 1; v < n; ++v) {
			int a = label[v], b = label[random() % v];
			graph[a].push_back(b); graph[b].push_back(a);
		}
		for (auto &neighbors : graph) std::shuffle(neighbors.begin(), neighbors.end(), random);
		check(graph);
	}
	Graph asymmetric(11);
	for (int v = 1; v <= 7; ++v) { asymmetric[0].push_back(v); asymmetric[v].push_back(0); }
	for (int v = 8; v < 11; ++v) { asymmetric[v - 1].push_back(v); asymmetric[v].push_back(v - 1); }
	check(asymmetric);
	assert((tree_center(asymmetric) == std::vector<int>{7, 8}));
	for (int n : {200000, 200001}) {
		Graph path(n), star(n);
		for (int v = 1; v < n; ++v) {
			path[v - 1].push_back(v); path[v].push_back(v - 1);
			star[0].push_back(v); star[v].push_back(0);
		}
		std::vector<int> expected = n % 2 ? std::vector<int>{n / 2} : std::vector<int>{n / 2 - 1, n / 2};
		assert(tree_center(path) == expected);
		assert((tree_center(star) == std::vector<int>{0}));
	}
}
