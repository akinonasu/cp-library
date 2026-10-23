#include "graph/topological_sort.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <numeric>
#include <queue>
#include <random>
#include <span>

using Graph = std::vector<std::vector<int>>;

template<class G>
bool valid(const G &graph, const std::vector<int> &order) {
	int n = graph.size();
	if (order.size() != std::size_t(n)) return false;
	std::vector<int> position(n, -1);
	for (int i = 0; i < n; ++i) {
		int v = order[i];
		if (v < 0 || v >= n || position[v] != -1) return false;
		position[v] = i;
	}
	for (int v = 0; v < n; ++v) for (int to : graph[v])
		if (position[v] >= position[to]) return false;
	return true;
}

std::optional<std::vector<int>> reference(const Graph &graph) {
	std::vector<int> order(graph.size());
	std::iota(order.begin(), order.end(), 0);
	do {
		if (valid(graph, order)) return order;
	} while (std::next_permutation(order.begin(), order.end()));
	return std::nullopt;
}

std::optional<std::vector<int>> priority_reference(const Graph &graph) {
	std::vector<std::size_t> indegree(graph.size());
	for (const auto &row : graph) for (int v : row) ++indegree[v];
	std::priority_queue<int, std::vector<int>, std::greater<int>> ready;
	for (int v = 0; v < int(graph.size()); ++v) if (!indegree[v]) ready.push(v);
	std::vector<int> result;
	while (!ready.empty()) {
		int v = ready.top(); ready.pop(); result.push_back(v);
		for (int to : graph[v]) if (!--indegree[to]) ready.push(to);
	}
	if (result.size() != graph.size()) return std::nullopt;
	return result;
}

void check(const Graph &graph) {
	auto original = graph;
	auto expected = reference(graph);
	auto order = topological_sort(graph);
	assert(order.has_value() == expected.has_value());
	if (order) assert(valid(graph, *order));
	assert(topological_sort<true>(graph) == expected);
	assert(topological_sort(graph) == order);
	assert(graph == original);
}

int main() {
	for (int n = 0; n <= 4; ++n) {
		for (int mask = 0; mask < (1 << (n * n)); ++mask) {
			Graph graph(n);
			for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v)
				if ((mask >> (u * n + v)) & 1) graph[u].push_back(v);
			check(graph);
		}
	}
	std::mt19937 rng(724610);
	for (int trial = 0; trial < 1000; ++trial) {
		int n = rng() % 8;
		Graph graph(n);
		for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v) {
			int count = rng() % 8 == 0 ? 1 + rng() % 4 : 0;
			while (count--) graph[u].push_back(v);
		}
		check(graph);
		for (auto &row : graph) std::shuffle(row.begin(), row.end(), rng);
		check(graph);
	}
	Graph different{ {}, {}, {0}, {1} };
	assert((topological_sort(different) == std::vector<int>{2, 3, 0, 1}));
	assert((topological_sort<true>(different) == std::vector<int>{2, 0, 3, 1}));
	Graph disconnected{ {1}, {}, {3}, {2}, {} };
	assert(!topological_sort(disconnected) && !topological_sort<true>(disconnected));
	std::array<std::vector<int>, 3> array{ std::vector<int>{2}, {}, {} };
	assert(valid(array, *topological_sort(array)));
	assert((topological_sort<true>(std::span(array)) == std::vector<int>{0, 1, 2}));
	std::vector<std::deque<int>> deques{ {1, 1}, {}, {1} };
	assert(valid(deques, *topological_sort(deques)));
	assert((topological_sort<true>(deques) == std::vector<int>{0, 2, 1}));
	for (int width : {31, 32, 33, 127, 128, 129, 257}) {
		const int n = 1 + 4 * (width + 1);
		std::vector<int> label(n);
		std::iota(label.begin(), label.end(), 0);
		std::shuffle(label.begin(), label.end(), rng);
		Graph layers(n);
		for (int block = 0; block < 4; ++block) {
			int before = block * (width + 1), after = before + width + 1;
			for (int v = before + 1; v < after; ++v) {
				layers[label[before]].push_back(label[v]);
				layers[label[v]].push_back(label[after]);
			}
		}
		assert(topological_sort<true>(layers) == priority_reference(layers));
	}
	for (int trial = 0; trial < 120; ++trial) {
		int n = 128 + rng() % 1921;
		Graph graph(n);
		std::vector<int> label(n);
		std::iota(label.begin(), label.end(), 0);
		std::shuffle(label.begin(), label.end(), rng);
		for (int i = 0, m = (trial % 9) * n; i < m; ++i) {
			int a = rng() % n, b = rng() % n;
			if (a > b) std::swap(a, b);
			if (a != b) graph[label[a]].push_back(label[b]);
		}
		assert(topological_sort<true>(graph) == priority_reference(graph));
		graph[label[0]].push_back(label[0]);
		assert(topological_sort<true>(graph) == priority_reference(graph));
	}

	const int n = 500000;
	Graph path(n);
	for (int v = n - 1; v > 0; --v) path[v].push_back(v - 1);
	for (bool lexicographical : {false, true}) {
		auto result = lexicographical ? topological_sort<true>(path) : topological_sort(path);
		assert(result && valid(path, *result));
		for (int i = 0; i < n; ++i) assert((*result)[i] == n - i - 1);
	}
	path[0].push_back(n - 1);
	assert(!topological_sort(path) && !topological_sort<true>(path));
	Graph wide(n);
	for (int v = 1; v < n; ++v) wide[n - 1].push_back(v - 1);
	auto order = topological_sort<true>(wide);
	assert(order && valid(wide, *order) && order->front() == n - 1);
	for (int i = 1; i < n; ++i) assert((*order)[i] == i - 1);
	Graph parallel(2);
	parallel[0].assign(1000000, 1);
	assert((topological_sort(parallel) == std::vector<int>{0, 1}));
	assert((topological_sort<true>(parallel) == std::vector<int>{0, 1}));
}
