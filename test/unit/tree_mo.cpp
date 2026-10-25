#include "data_structure/range_query/tree_mo.hpp"
#include <numeric>
#include <random>
#include <queue>

void check(const std::vector<std::vector<int>> &graph, int root) {
	int n = graph.size();
	for (bool edge : {false, true}) {
		TreeMo mo(graph, root, edge);
		std::vector<int> parent(n, -2), depth(n), queue{root};
		parent[root] = -1;
		for (int i = 0; i < n; ++i) for (int to : graph[queue[i]]) if (to != parent[queue[i]]) {
			parent[to] = queue[i]; depth[to] = depth[queue[i]] + 1; queue.push_back(to);
		}
		std::vector<std::vector<unsigned char>> expected;
		for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v) {
			assert(mo.add_query(u, v) == int(expected.size()));
			std::vector<unsigned char> path(n);
			int a = u, b = v;
			while (a != b) {
				if (depth[a] < depth[b]) std::swap(a, b);
				path[a] = 1;
				a = parent[a];
			}
			if (!edge) path[a] = 1;
			expected.push_back(path);
		}
		for (int block : {0, 1, 5, 10000}) {
			std::vector<unsigned char> active(n);
			std::vector<int> seen(expected.size());
			mo.run([&](int v) { assert(!active[v]); active[v] = 1; },
				[&](int v) { assert(active[v]); active[v] = 0; },
				[&](int id) { assert(!seen[id]++ && active == expected[id]); }, block);
			for (int x : seen) assert(x == 1);
		}
		for (int v = 0; v < n; ++v) assert(mo.parent(v) == parent[v]);
	}
}

int main() {
	TreeMo empty({});
	assert(empty.size() == 0 && empty.order().empty());
	empty.run([](int) { assert(false); }, [](int) { assert(false); }, [](int) { assert(false); });
	std::mt19937 rng(968763);
	for (int n : {1, 2, 3, 8, 17, 40}) for (int shape = 0; shape < 3; ++shape) for (int trial = 0; trial < 5; ++trial) {
		std::vector<int> labels(n);
		std::iota(labels.begin(), labels.end(), 0);
		std::shuffle(labels.begin(), labels.end(), rng);
		std::vector<std::vector<int>> graph(n);
		for (int v = 1; v < n; ++v) {
			int p = shape == 0 ? v - 1 : shape == 1 ? 0 : rng() % v;
			graph[labels[v]].push_back(labels[p]); graph[labels[p]].push_back(labels[v]);
		}
		for (auto &row : graph) std::shuffle(row.begin(), row.end(), rng);
		check(graph, rng() % n);
	}
	int n = 200000;
	std::vector<std::vector<int>> graph(n);
	for (int v = 1; v < n; ++v) graph[v - 1].push_back(v), graph[v].push_back(v - 1);
	TreeMo line(graph, n - 1);
	line.add_query(0, n - 1); line.add_query(n / 2, n / 2);
	int count = 0;
	line.run([&](int) { ++count; }, [&](int) { --count; }, [&](int id) { assert(count == (id ? 1 : n)); });
}
