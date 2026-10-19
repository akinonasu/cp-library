#include "data_structure/advanced/centroid_decomposition.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

struct Baseline {
	using Edge = CentroidDecomposition<>::Edge;
	using Ancestor = CentroidDecomposition<>::Ancestor;
	int n;
	std::vector<Edge> edges;
	std::vector<int> parent, depth, size, in, out, order;
	std::vector<std::vector<Ancestor>> paths;
	explicit Baseline(int n) : n(n) { edges.reserve(n - 1); }
	void add_edge(int u, int v) { edges.push_back({u, v, 1}); }
	void build(bool distances) {
		std::vector<std::vector<int>> graph(n);
		for (auto e : edges) graph[e.u].push_back(e.v), graph[e.v].push_back(e.u);
		parent.assign(n, -1), depth.assign(n, -1), size.resize(n), in.resize(n), out.resize(n);
		if (distances) paths.resize(n);
		std::vector<int> previous(n), sub(n), work, distance(distances ? n : 0);
		work.reserve(n), order.reserve(n);
		auto decompose = [&](auto&& self, int s, int p) -> void {
			work.clear(), work.push_back(s), previous[s] = -1;
			for (std::size_t i = 0; i < work.size(); ++i) {
				int v = work[i];
				sub[v] = 1;
				for (int u : graph[v]) if (u != previous[v] && depth[u] == -1)
					previous[u] = v, work.push_back(u);
			}
			int count = int(work.size()), c = -1;
			for (std::size_t i = work.size(); i-- > 0;) {
				int v = work[i];
				if (c == -1 && sub[v] >= (count + 1) / 2) c = v;
				if (previous[v] != -1) sub[previous[v]] += sub[v];
			}
			parent[c] = p, depth[c] = p == -1 ? 0 : depth[p] + 1;
			size[c] = count, in[c] = int(order.size()), out[c] = in[c] + count;
			order.push_back(c);
			if (distances) {
				work.clear(), work.push_back(c), previous[c] = -1, distance[c] = 0;
				for (std::size_t i = 0; i < work.size(); ++i) {
					int v = work[i];
					paths[v].push_back({c, -1, distance[v]});
					for (int u : graph[v]) if (u != previous[v] && depth[u] == -1)
						previous[u] = v, distance[u] = distance[v] + 1, work.push_back(u);
				}
			}
			for (int u : graph[c]) if (depth[u] == -1) self(self, u, c);
		};
		decompose(decompose, 0, -1);
		if (distances) for (auto& path : paths)
			for (std::size_t i = 0; i + 1 < path.size(); ++i) path[i].branch = path[i + 1].centroid;
	}
	std::span<const Ancestor> ancestors(int v) const { return paths[v]; }
};

template<class Tree> void run(const std::string& shape, int n, bool distances) {
	Tree tree(n);
	std::mt19937 rng(731);
	std::vector<int> label(n);
	for (int i = 0; i < n; ++i) label[i] = i;
	if (shape == "random_labels") std::shuffle(label.begin(), label.end(), rng);
	for (int v = 1; v < n; ++v) {
		int p = shape == "path" ? v - 1 : shape == "star" ? 0 : shape == "balanced" ? (v - 1) / 2
			: shape == "broom" ? (v < n / 2 ? v - 1 : n / 2 - 1) : rng() % v;
		tree.add_edge(label[p], label[v]);
	}
	auto start = std::chrono::steady_clock::now();
	tree.build(distances);
	auto end = std::chrono::steady_clock::now();
	std::uint64_t hash = 0;
	auto mix = [&](int x) { hash = (hash ^ std::uint32_t(x)) * 0x9e3779b97f4a7c15ULL; };
	for (int v = 0; v < n; ++v) {
		mix(tree.parent[v]), mix(tree.depth[v]), mix(tree.size[v]), mix(tree.in[v]), mix(tree.out[v]);
		if (distances) for (auto a : tree.ancestors(v)) mix(a.centroid), mix(a.branch), mix(a.distance);
	}
	std::cout << std::setprecision(12) << std::chrono::duration<double>(end - start).count() << '\n' << hash << '\n';
}

int main(int argc, char** argv) {
	if (argc != 5) return 1;
	if (std::string(argv[1]) == "baseline") run<Baseline>(argv[2], std::atoi(argv[3]), std::atoi(argv[4]));
	else run<CentroidDecomposition<>>(argv[2], std::atoi(argv[3]), std::atoi(argv[4]));
}
