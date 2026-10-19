#include "tree/static_top_tree.hpp"
#include "test/static_top_tree_dp.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

using DP = StaticTopTreeRerootDP;

mint naive(const HLD& tree, const std::vector<DP::Vertex>& values, int root, bool subtree) {
	std::vector<std::vector<std::pair<int, int>>> adj(tree.n);
	for (int e = 0; e < tree.n - 1; ++e) {
		auto [u, v] = tree.edges[e];
		int child = tree.parent[u] == v ? u : v;
		adj[u].emplace_back(v, child), adj[v].emplace_back(u, child);
	}
	std::vector<int> parent(tree.n, -1), order{root};
	std::vector<mint> a(tree.n, 1), b(tree.n);
	mint answer = 0;
	for (std::size_t i = 0; i < order.size(); ++i) {
		int v = order[i];
		answer += a[v] * values[v].value + b[v];
		for (auto [u, child] : adj[v]) {
			if (u == parent[v] || (subtree && u == tree.parent[root] && v == root)) continue;
			parent[u] = v;
			a[u] = a[v] * values[child].a, b[u] = a[v] * values[child].b + b[v];
			order.push_back(u);
		}
	}
	return answer;
}

struct IndependentSetDP {
	using Vertex = mint;
	using Point = std::array<mint, 2>;
	using Path = std::array<mint, 4>;
	static Point point_identity() { return {1, 1}; }
	static Path path_identity() { return {1, 0, 0, 1}; }
	static Point rake(const Point& a, const Point& b) { return {a[0] * b[0], a[1] * b[1]}; }
	static Point add_edge(const Path& a) { return {a[0] + a[2], a[0]}; }
	static Path add_vertex(const Point& p, Vertex value) { return {p[0], p[0], value * p[1], 0}; }
	static Path compress(const Path& a, const Path& b) {
		return {a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3],
			a[2] * b[0] + a[3] * b[2], a[2] * b[1] + a[3] * b[3]};
	}
};

void verify_structure(const HLD& tree) {
	static_top_tree_detail::Structure structure(tree);
	int n = tree.n;
	assert(structure.nodes.size() <= std::size_t(std::max(0, 2 * n - 1)));
	assert(structure.order.size() == structure.nodes.size());
	std::vector<int> depth(structure.nodes.size()), seen(structure.nodes.size());
	int max_depth = 0;
	for (int v : structure.order) {
		const auto& node = structure.nodes[v];
		assert(!seen[v]++);
		for (int u : {node.left, node.right}) if (u != -1) {
			assert(seen[u] && structure.nodes[u].parent == v);
			depth[v] = std::max(depth[v], depth[u] + 1);
		}
		max_depth = std::max(max_depth, depth[v]);
	}
	assert(max_depth == structure.height);
	assert(max_depth <= 6 * int(std::bit_width(unsigned(n))));
	if (n) assert(structure.nodes[structure.root].parent == -1);
}

int main() {
	StaticTopTree<DP> dp;
	assert(dp.empty() && dp.size() == 0 && dp.root() == -1 && dp.dp().sum == mint(0));
	HLD empty;
	empty.build();
	dp.build(empty, {});
	verify_structure(empty);
	std::mt19937 random(761825);
	for (int n = 1; n <= 90; ++n) for (int trial = 0; trial < 6; ++trial) {
		HLD tree(n);
		for (int v = 1; v < n; ++v) {
			int u = trial == 0 ? v - 1 : trial == 1 ? 0 : trial == 2 ? (v - 1) / 2 : int(random() % v);
			if (random() & 1) tree.add_edge(u, v);
			else tree.add_edge(v, u);
		}
		int root = int(random() % n);
		tree.build(root);
		verify_structure(tree);
		std::vector<DP::Vertex> values(n);
		for (auto& v : values) v = {random() % 19, random() % 7, random() % 11};
		values[root].a = 1, values[root].b = 0;
		dp.build(tree, values);
		assert(!dp.empty() && dp.size() == n && dp.root() == root);
		for (int v = 0; v < n; ++v) assert(dp.parent(v) == tree.parent[v]);
		for (int e = 0; e < n - 1; ++e) {
			auto [u, v] = tree.edges[e];
			assert(dp.edge_vertex(e) == (tree.parent[u] == v ? u : v));
		}
		for (int step = 0; step < 24; ++step) {
			int v = int(random() % n);
			values[v].value = random() % 19;
			if (v != root && (step & 1)) values[v].a = random() % 7, values[v].b = random() % 11;
			dp.set(v, values[v]);
			assert(dp.get(v).value == values[v].value);
			assert(dp.dp().sum == naive(tree, values, root, false));
			for (int k = 0; k < 4; ++k) {
				int r = int(random() % n);
				assert(dp.prod(r).reverse.sum == naive(tree, values, r, false));
				auto p = dp.subtree_prod(r).forward;
				mint expected = naive(tree, values, r, true);
				assert(p.sum == values[r].a * expected + values[r].b * tree.size[r]);
			}
		}
		std::vector<mint> weights(n);
		for (auto& w : weights) w = random() % 5;
		StaticTopTree<IndependentSetDP> independent(tree, weights);
		for (int step = 0; step < 5; ++step) {
			int v = int(random() % n); weights[v] = random() % 5;
			independent.set(v, weights[v]);
			std::vector<std::array<mint, 2>> ways(n);
			for (int i = n - 1; i >= 0; --i) {
				int u = tree.order[i];
				ways[u] = {1, weights[u]};
				for (int j = tree.in[u] + 1; j < tree.out[u]; j = tree.out[tree.order[j]]) {
					auto child = ways[tree.order[j]];
					ways[u][0] *= child[0] + child[1], ways[u][1] *= child[0];
				}
				assert(independent.subtree_prod(u)[0] == ways[u][0]);
			}
			assert(independent.dp()[0] == ways[root][0] + ways[root][1]);
		}
	}
	for (int shape = 0; shape < 5; ++shape) {
		int n = 300000;
		HLD tree(n);
		for (int v = 1; v < n; ++v) {
			int p = shape == 0 ? v - 1 : shape == 1 ? 0 : shape == 2 ? (v - 1) / 2 :
				shape == 3 ? (v < n / 2 ? v - 1 : v - n / 2) : int(random() % v);
			tree.add_edge(p, v);
		}
		tree.build(shape == 3 ? n - 1 : 0);
		verify_structure(tree);
		std::vector<DP::Vertex> values(n, {1, 1, 0});
		dp.build(tree, std::move(values));
		assert(dp.dp().sum == mint(n));
		for (int step = 0; step < 30; ++step) {
			int v = int(random() % n);
			auto value = dp.get(v); value.value += 1;
			dp.set(v, value);
			assert(dp.prod(int(random() % n)).reverse.sum == mint(n + step + 1));
		}
	}
	dp.build(empty, {});
	assert(dp.empty() && dp.root() == -1 && dp.all_prod().forward.count == mint(0));
}
