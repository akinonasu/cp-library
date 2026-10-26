#include "tree/top_tree.hpp"
#include "test/fixtures/top_tree_dp.hpp"
#include <cassert>
#include <random>
#include <vector>

int main() {
	TopTree<TopTreeSumDP> empty;
	assert(empty.empty() && empty.size() == 0);
	TopTree<TopTreeSumDP> zero(std::vector<std::int64_t>{});
	assert(zero.empty());
	std::mt19937 rng(715238);
	for (int trial = 0; trial < 80; ++trial) {
		int n = 1 + rng() % 35;
		std::vector<std::int64_t> values(n);
		std::vector<TopTreeCompositeDP::Vertex> functions(n);
		for (int v = 0; v < n; ++v) values[v] = rng() % 30, functions[v] = {rng() % 20, rng() % 20};
		TopTree<TopTreeSumDP> sum(values);
		TopTree<TopTreeDiameterDP> diameter(values);
		TopTree<TopTreeCompositeDP> composite(functions);
		std::vector<std::vector<bool>> edge(n, std::vector<bool>(n));
		auto parents = [&](int root) {
			std::vector<int> p(n, -1), order{root};
			p[root] = root;
			for (int i = 0; i < int(order.size()); ++i) {
				int v = order[i];
				for (int u = 0; u < n; ++u) if (edge[v][u] && p[u] < 0) p[u] = v, order.push_back(u);
			}
			return p;
		};
		for (int step = 0; step < 2500; ++step) {
			int u = rng() % n, v = rng() % n, kind = rng() % 6;
			auto p = parents(u);
			if (kind == 0) {
				bool ok = p[v] < 0;
				assert(sum.link(u, v) == ok);
				assert(diameter.link(u, v) == ok);
				assert(composite.link(u, v) == ok);
				if (ok) edge[u][v] = edge[v][u] = true;
			} else if (kind == 1) {
				bool ok = edge[u][v];
				assert(sum.cut(u, v) == ok);
				assert(diameter.cut(u, v) == ok);
				assert(composite.cut(u, v) == ok);
				edge[u][v] = edge[v][u] = false;
			} else if (kind == 2) {
				values[u] = rng() % 30, functions[u] = {rng() % 20, rng() % 20};
				sum.set(u, values[u]), diameter.set(u, values[u]), composite.set(u, functions[u]);
				assert(sum.get(u) == values[u] && composite.get(u) == functions[u]);
			} else if (kind == 3) {
				assert(sum.same(u, v) == (p[v] >= 0));
				assert(diameter.same(u, v) == (p[v] >= 0));
				assert(composite.same(u, v) == (p[v] >= 0));
			} else {
				std::int64_t total = 0, best = 0;
				for (int a = 0; a < n; ++a) if (p[a] >= 0) {
					total += values[a];
					auto q = parents(a);
					for (int b = 0; b < n; ++b) if (q[b] >= 0) {
						std::int64_t distance = values[a];
						for (int x = b; x != a; x = q[x]) distance += values[x];
						best = std::max(best, distance);
					}
				}
				assert(sum.all_prod(u) == total);
				assert(diameter.all_prod(u).diameter == best);
				assert(composite.all_prod(u) == functions[u]);
				if (p[v] < 0) continue;
				assert(sum.prod(u, v) == total);
				assert(diameter.prod(u, v).diameter == best);
				std::vector<int> path;
				for (int x = v; x != u; x = p[x]) path.push_back(x);
				path.push_back(u);
				TopTreeCompositeDP::Path f{1, 0};
				for (auto it = path.rbegin(); it != path.rend(); ++it) f = TopTreeCompositeDP::compress(f, functions[*it]);
				assert(composite.prod(u, v) == f);
				std::int64_t subtree = 0;
				for (int a = 0; a < n; ++a) if (p[a] >= 0) {
					int x = a;
					while (x != u && x != v) x = p[x];
					if (x == v) subtree += values[a];
				}
				assert(sum.subtree_prod(u, v) == subtree);
				assert(composite.subtree_prod(u, v) == functions[v]);
			}
		}
		auto copy = sum;
		assert(copy.all_prod(0) == sum.all_prod(0));
		copy.set(0, copy.get(0) + 1);
		assert(copy.all_prod(0) == sum.all_prod(0) + 1);
	}
	for (bool star : {false, true}) {
		int n = 100000;
		TopTree<TopTreeSumDP> sum(std::vector<std::int64_t>(n, 1));
		for (int v = 1; v < n; ++v) assert(sum.link(v, star ? 0 : v - 1));
		for (int v = 0; v < n; v += 31) {
			assert(sum.all_prod(v) == n);
			assert(sum.subtree_prod(0, v) == (star ? (v ? 1 : n) : n - v));
		}
		for (int v = 1; v < n; ++v) assert(sum.cut(v, star ? 0 : v - 1));
		for (int v = 0; v < n; ++v) assert(sum.all_prod(v) == 1);
	}
	for (int trial = 0; trial < 60; ++trial) {
		int n = 1 + rng() % 11;
		std::vector<mint> weights(n, 1);
		TopTree<TopTreeIndependentSetDP> tree(weights);
		std::vector<std::pair<int, int>> edges;
		for (int v = 1; v < n; ++v) {
			int u = rng() % v;
			edges.emplace_back(u, v);
			assert(tree.link(u, v));
		}
		for (int step = 0; step < 80; ++step) {
			int v = rng() % n;
			weights[v] = rng() % 4;
			tree.set(v, weights[v]);
			if (!edges.empty()) {
				int i = rng() % edges.size();
				auto [a, b] = edges[i];
				assert(tree.cut(a, b));
				int x, y;
				do x = rng() % n, y = rng() % n; while (tree.same(x, y));
				assert(tree.link(x, y));
				edges[i] = {x, y};
			}
			mint expected = 0;
			for (int mask = 0; mask < (1 << n); ++mask) {
				bool ok = true;
				for (auto [a, b] : edges) if ((mask >> a & 1) && (mask >> b & 1)) ok = false;
				if (!ok) continue;
				mint value = 1;
				for (int u = 0; u < n; ++u) if (mask >> u & 1) value *= weights[u];
				expected += value;
			}
			for (int root : {0, int(rng() % n), n - 1}) {
				auto p = tree.all_prod(root);
				assert(p[0] + p[2] == expected);
			}
		}
	}
}
