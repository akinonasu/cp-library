#include "tree/centroid_decomposition.hpp"
#include <cstdint>
#include <random>

template<class T> void verify(CentroidDecomposition<T>& tree) {
	int n = tree.n;
	tree.build();
	assert(tree.has_distances());
	if (!n) { assert(tree.root == -1 && tree.order.empty()); return; }
	std::vector<std::vector<std::pair<int, T>>> graph(n);
	for (auto e : tree.edges) graph[e.u].emplace_back(e.v, e.weight), graph[e.v].emplace_back(e.u, e.weight);
	std::vector<std::vector<T>> distance(n, std::vector<T>(n));
	for (int s = 0; s < n; ++s) {
		std::vector<int> parent(n, -1), work{s};
		parent[s] = s;
		for (std::size_t i = 0; i < work.size(); ++i) {
			int v = work[i];
			for (auto [u, w] : graph[v]) if (u != parent[v]) {
				parent[u] = v, distance[s][u] = distance[s][v] + w, work.push_back(u);
			}
		}
	}
	assert(tree.parent[tree.root] == -1 && tree.depth[tree.root] == 0);
	assert(tree.size[tree.root] == n && tree.order[0] == tree.root);
	std::vector<int> seen(n), subtree_size(n, 1);
	for (int i = n - 1; i >= 0; --i) {
		int v = tree.order[i];
		assert(0 <= v && v < n && !seen[v]++ && tree.in[v] == i);
		assert(tree.out[v] == i + tree.size[v] && tree.out[v] <= n);
		assert(tree.subtree(v) == std::make_pair(i, i + tree.size[v]));
		assert(tree.size[v] == subtree_size[v]);
		int p = tree.parent[v];
		if (p != -1) {
			assert(tree.depth[v] == tree.depth[p] + 1 && 2 * tree.size[v] <= tree.size[p]);
			subtree_size[p] += subtree_size[v];
		}
		std::vector<int> chain;
		for (int c = v; c != -1; c = tree.parent[c]) chain.push_back(c);
		auto path = tree.ancestors(v);
		assert(path.size() == chain.size());
		for (std::size_t j = 0; j < chain.size(); ++j) {
			const auto& a = path[chain.size() - 1 - j];
			assert(a.centroid == chain[j] && a.distance == distance[v][a.centroid]);
			assert(a.branch == (j ? chain[j - 1] : -1));
			assert(tree.distance_to_ancestor(v, a.centroid) == a.distance);
		}
		std::size_t j = 0;
		tree.for_each_ancestor(v, [&](int c, T d, int b) {
			assert(c == chain[j] && d == distance[v][c] && b == (j ? chain[j - 1] : -1));
			++j;
		});
		assert(j == chain.size());
		for (int u = 0; u < n; ++u) {
			bool ancestor = std::find(chain.begin(), chain.end(), u) != chain.end();
			assert(tree.is_ancestor(u, v) == ancestor);
			int c = u;
			while (std::find(chain.begin(), chain.end(), c) == chain.end()) c = tree.parent[c];
			assert(tree.centroid_lca(u, v) == c && tree.dist(u, v) == distance[u][v]);
		}
		std::vector<int> work{v}, visited(n);
		visited[v] = 1;
		for (std::size_t k = 0; k < work.size(); ++k)
			for (auto [u, w] : graph[work[k]]) if (!visited[u] && tree.is_ancestor(v, u)) {
				(void)w;
				visited[u] = 1, work.push_back(u);
			}
		assert(int(work.size()) == tree.size[v]);
	}
	auto parent = tree.parent, depth = tree.depth, order = tree.order;
	tree.build(false);
	assert(!tree.has_distances() && tree.parent == parent && tree.depth == depth && tree.order == order);
	for (int v = 0; v < n; ++v) assert(tree.centroid_lca(v, tree.root) == tree.root);
	tree.build();
	assert(tree.has_distances() && tree.parent == parent && tree.dist(0, n - 1) == distance[0][n - 1]);
}

int main() {
	CentroidDecomposition<> empty;
	verify(empty);
	empty.build(false);
	assert(!empty.has_distances());
	CentroidDecomposition<> single(1);
	verify(single);
	std::mt19937 rng(731);
	for (int trial = 0; trial < 220; ++trial) {
		int n = 2 + rng() % 99;
		CentroidDecomposition<> tree(n);
		CentroidDecomposition<long long> weighted(n);
		for (int v = 1; v < n; ++v) {
			int p = trial % 5 == 0 ? v - 1 : trial % 5 == 1 ? 0 : trial % 5 == 2 ? (v - 1) / 2 : rng() % v;
			assert(tree.add_edge(p, v) == v - 1);
			weighted.add_edge(p, v, (static_cast<long long>(rng() % 2001) - 1000) * 1000000000LL);
		}
		verify(tree), verify(weighted);
		std::vector<int> best(n, n), active(n);
		for (int q = 0; q < 200; ++q) {
			int v = rng() % n;
			if (q % 2 == 0) {
				active[v] = 1;
				tree.for_each_ancestor(v, [&](int c, int d, int) { best[c] = std::min(best[c], d); });
			} else {
				int actual = n, expected = n;
				tree.for_each_ancestor(v, [&](int c, int d, int) { actual = std::min(actual, best[c] + d); });
				for (int u = 0; u < n; ++u) if (active[u]) expected = std::min(expected, tree.dist(u, v));
				assert(actual == expected);
			}
		}
	}
	for (bool star : {false, true}) {
		constexpr int n = 300000;
		CentroidDecomposition<> tree(n);
		for (int v = 1; v < n; ++v) tree.add_edge(star ? 0 : v - 1, v);
		tree.build();
		for (int v = 0; v < n; ++v) {
			assert(tree.depth[v] < 19 && tree.out[v] - tree.in[v] == tree.size[v]);
			assert(tree.dist(0, v) == (star ? int(v != 0) : v));
		}
	}
	CentroidDecomposition<double> real(3);
	real.add_edge(0, 1, 0.5), real.add_edge(1, 2, 1.25);
	real.build();
	assert(real.dist(0, 2) == 1.75);
}
