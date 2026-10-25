#include "tree/auxiliary_tree.hpp"
#include "tree/lca.hpp"
#include "data_structure/hld.hpp"
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <type_traits>

void require(bool ok) {
	if (!ok) std::abort();
}

int naive_lca(const std::vector<int> &parent, const std::vector<int> &depth, int u, int v) {
	while (depth[u] > depth[v]) u = parent[u];
	while (depth[v] > depth[u]) v = parent[v];
	while (u != v) u = parent[u], v = parent[v];
	return u;
}

template<class Tree>
void check(AuxiliaryTree<Tree> &vt, const Tree &tree, const std::vector<int> &keys,
		   const std::vector<int> &parent, const std::vector<int> &depth) {
	auto original = keys;
	const auto &nodes = vt.build(keys);
	require(keys == original && &nodes == &vt.nodes());
	std::set<int> chosen(keys.begin(), keys.end()), closure = chosen;
	for (int u : chosen) for (int v : chosen) closure.insert(naive_lca(parent, depth, u, v));
	require(vt.size() == static_cast<int>(closure.size()));
	require(vt.empty() == keys.empty() && vt.root() == vt.size() - 1);
	std::vector<int> index(parent.size(), -1), vp(vt.size()), vd(vt.size());
	for (int i = 0; i < vt.size(); ++i) {
		const auto &node = vt[i];
		require(&node == &nodes[i] && closure.count(node.vertex) && index[node.vertex] == -1);
		index[node.vertex] = i;
		require(node.selected == chosen.contains(node.vertex));
		if (i == vt.root()) require(node.parent == -1 && node.length == 0);
		else {
			require(i < node.parent && node.parent < vt.size());
			int p = parent[node.vertex];
			while (!closure.contains(p)) p = parent[p];
			require(nodes[node.parent].vertex == p);
			require(node.length == depth[node.vertex] - depth[p] && node.length > 0);
		}
		vp[i] = node.parent;
	}
	const auto &view = vt;
	auto graph = view.to_graph();
	require(graph.size() == nodes.size());
	std::vector<int> graph_parent(vt.size(), -2), order;
	if (!vt.empty()) graph_parent[vt.root()] = -1, order.push_back(vt.root());
	std::size_t arcs = 0;
	for (std::size_t i = 0; i < order.size(); ++i) {
		int v = order[i], upward = 0;
		arcs += graph[v].size();
		for (int to : graph[v]) {
			require(0 <= to && to < vt.size());
			if (to == graph_parent[v]) { ++upward; continue; }
			require(graph_parent[to] == -2);
			graph_parent[to] = v;
			order.push_back(to);
		}
		require(upward == (v == vt.root() ? 0 : 1));
	}
	require(order.size() == nodes.size() && graph_parent == vp);
	require(arcs == (vt.empty() ? 0 : 2 * std::size_t(vt.size() - 1)));
	for (int i = vt.size() - 2; i >= 0; --i) vd[i] = vd[vp[i]] + 1;
	for (int u : closure) for (int v : closure) {
		int w = naive_lca(vp, vd, index[u], index[v]);
		require(vt[w].vertex == naive_lca(parent, depth, u, v));
	}
	std::vector<long long> count(vt.size());
	long long sum = 0;
	for (int i = 0; i < vt.size(); ++i) {
		count[i] += vt[i].selected;
		if (vt[i].parent >= 0) {
			sum += count[i] * (static_cast<int>(chosen.size()) - count[i]) * vt[i].length;
			count[vt[i].parent] += count[i];
		}
	}
	long long expected = 0;
	for (int u : chosen) for (int v : chosen) if (u < v)
		expected += depth[u] + depth[v] - 2 * depth[naive_lca(parent, depth, u, v)];
	require(sum == expected);
	auto saved = nodes;
	std::vector<int> sorted = keys;
	std::sort(sorted.begin(), sorted.end(), [&](int u, int v) { return tree.subtree(u).first < tree.subtree(v).first; });
	vt.build_sorted(sorted);
	require(vt.size() == static_cast<int>(saved.size()));
	for (int i = 0; i < vt.size(); ++i)
		require(vt[i].vertex == saved[i].vertex && vt[i].parent == saved[i].parent &&
			vt[i].length == saved[i].length && vt[i].selected == saved[i].selected);
}

int main() {
	static_assert(!std::is_constructible_v<AuxiliaryTree<LCA>, LCA &&>);
	LCA empty;
	empty.build();
	AuxiliaryTree empty_vt(empty);
	require(empty_vt.to_graph().empty());
	require(empty_vt.build({}).empty() && empty_vt.root() == -1);
	require(empty_vt.to_graph().empty());
	{
		LCA tree(5);
		for (auto [u, v] : std::vector<std::pair<int, int>>{ {0, 1}, {1, 2}, {1, 3}, {3, 4} }) tree.add_edge(u, v);
		tree.build();
		AuxiliaryTree vt(tree);
		vt.build(std::vector<int>{2, 4});
		auto graph = vt.to_graph();
		require(graph.size() == 3);
		auto saved = graph;
		graph[vt.root()].clear();
		require(vt.to_graph() == saved);
		vt.build(std::vector<int>{4});
		require(vt.to_graph() == std::vector<std::vector<int>>(1));
		require(saved.size() == 3 && !saved.back().empty());
		vt.build({});
		require(vt.to_graph().empty() && !saved.back().empty());
	}
	std::mt19937 rng(413708);
	{
		LCA tree(8192);
		for (int v = 1; v < tree.n; ++v) tree.add_edge(rng() % v, v);
		tree.build(731);
		AuxiliaryTree vt(tree);
		for (int size : {255, 256, 257, 511, 512, 1023, 1024, 8192}) for (int shape = 0; shape < 3; ++shape) {
			std::vector<int> keys(size);
			for (int &v : keys) v = rng() % (shape == 0 ? 1 : shape == 1 ? 256 : tree.n);
			auto original = keys;
			auto saved = vt.build(keys);
			require(keys == original);
			std::sort(keys.begin(), keys.end(), [&](int u, int v) { return tree.subtree(u).first < tree.subtree(v).first; });
			vt.build_sorted(keys);
			require(vt.size() == int(saved.size()));
			for (int i = 0; i < vt.size(); ++i)
				require(vt[i].vertex == saved[i].vertex && vt[i].parent == saved[i].parent &&
					vt[i].length == saved[i].length && vt[i].selected == saved[i].selected);
		}
	}
	for (int n = 1; n <= 36; ++n) for (int trial = 0; trial < 8; ++trial) {
		LCA lca(n);
		HLD hld(n);
		std::vector<int> labels(n);
		std::iota(labels.begin(), labels.end(), 0);
		std::shuffle(labels.begin(), labels.end(), rng);
		std::vector<std::pair<int, int>> edges;
		for (int v = 1; v < n; ++v) edges.emplace_back(labels[rng() % v], labels[v]);
		std::shuffle(edges.begin(), edges.end(), rng);
		std::vector<std::vector<int>> adj(n);
		for (auto [u, v] : edges) {
			lca.add_edge(u, v), hld.add_edge(u, v);
			adj[u].push_back(v), adj[v].push_back(u);
		}
		AuxiliaryTree a(lca);
		AuxiliaryTree b(hld);
		for (int root : {labels[0], static_cast<int>(rng() % n)}) {
			lca.build(root), hld.build(root);
			std::vector<int> parent(n, -1), depth(n), queue{root};
			for (int i = 0; i < n; ++i) for (int to : adj[queue[i]]) if (to != parent[queue[i]]) {
				parent[to] = queue[i], depth[to] = depth[queue[i]] + 1;
				queue.push_back(to);
			}
			for (int v = 0; v < n; ++v) {
				auto [l, r] = lca.subtree(v);
				int descendants = 0;
				for (int u = 0; u < n; ++u) {
					bool inside = naive_lca(parent, depth, u, v) == v;
					int pos = lca.subtree(u).first;
					require(inside == (l <= pos && pos < r));
					descendants += inside;
				}
				require(r - l == descendants);
			}
			int queries = n <= 8 ? 1 << n : 24;
			for (int q = 0; q < queries; ++q) {
				std::vector<int> keys;
				if (n <= 8) {
					for (int v = 0; v < n; ++v) if (q >> v & 1) keys.push_back(v);
				} else {
					int k = rng() % (2 * n + 1);
					for (int i = 0; i < k; ++i) keys.push_back(rng() % n);
				}
				std::shuffle(keys.begin(), keys.end(), rng);
				check(a, lca, keys, parent, depth);
				check(b, hld, keys, parent, depth);
			}
		}
		auto copy = a;
		a.build({});
		require(copy.size() == b.size());
	}
	const int n = 300000;
	for (int shape = 0; shape < 3; ++shape) {
		LCA tree(n);
		for (int v = 1; v < n; ++v) tree.add_edge(shape == 0 ? v - 1 : shape == 1 ? 0 : (v - 1) / 2, v);
		tree.build();
		AuxiliaryTree vt(tree);
		std::vector<int> keys(n);
		std::iota(keys.begin(), keys.end(), 0);
		std::shuffle(keys.begin(), keys.end(), rng);
		vt.build(keys);
		require(vt.size() == n && vt[vt.root()].vertex == 0);
		for (int i = 0; i < vt.size() - 1; ++i)
			require(vt[i].selected && vt[i].length == 1 && vt[vt[i].parent].vertex == tree.parent[vt[i].vertex]);
		for (int q = 0; q < 30000; ++q) {
			int u = rng() % n, v = rng() % n;
			std::vector<int> pair{u, v, u};
			vt.build(pair);
			require(vt[vt.root()].vertex == tree.lca(u, v) && vt.size() <= 3);
		}
		vt.build({});
		require(vt.empty());
	}
	std::cout << "Auxiliary Tree OK\n";
}
