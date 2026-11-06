#ifndef CENTROID_DECOMPOSITION_HPP
#define CENTROID_DECOMPOSITION_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <span>
#include <utility>
#include <vector>

template<class T = int> class CentroidDecomposition {
	bool built_ = false, distances_ = false;
	std::vector<std::size_t> offsets_;
	void check([[maybe_unused]] int v) const { assert(built_ && 0 <= v && v < n); }

  public:
	struct Edge { int u, v; T weight; };
	struct Ancestor { int centroid, branch; T distance; };

  private:
	std::vector<Ancestor> paths_;

  public:
	int n, root = -1;
	std::vector<Edge> edges;
	std::vector<int> parent, depth, size, in, out, order;
	explicit CentroidDecomposition(int n = 0) : n(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		edges.reserve(std::max(0, n - 1));
	}
	int add_edge(int u, int v, T weight = T(1)) {
		assert(0 <= u && u < n && 0 <= v && v < n && u != v);
		built_ = distances_ = false;
		edges.push_back({u, v, weight});
		return int(edges.size()) - 1;
	}
	void build(bool distances = true) {
		assert(n >= 0 && edges.size() == std::size_t(std::max(0, n - 1)));
		built_ = distances_ = false;
		root = -1;
		parent.assign(n, -1), depth.assign(n, -1), size.resize(n);
		in.resize(n), out.resize(n), order.clear();
		order.reserve(n);
		if (!distances) {
			std::vector<Ancestor>().swap(paths_);
			std::vector<std::size_t>().swap(offsets_);
		}
		if (!n) {
			paths_.clear(), offsets_.assign(1, 0);
			built_ = true, distances_ = distances;
			return;
		}
		std::vector<int> start(n + 1), adj(2 * edges.size());
		std::vector<T> weights(distances ? adj.size() : 0);
		for (const auto& e : edges) ++start[e.u + 1], ++start[e.v + 1];
		for (int v = 0; v < n; ++v) start[v + 1] += start[v];
		{
			auto cursor = start;
			for (const auto& e : edges) {
				int a = cursor[e.u]++, b = cursor[e.v]++;
				adj[a] = e.v, adj[b] = e.u;
				if (distances) weights[a] = weights[b] = e.weight;
			}
		}
		std::vector<int> previous(n, -1), sub(n), work;
		std::vector<std::pair<int, int>> tasks;
		work.reserve(n);
		tasks.emplace_back(0, -1);
		while (!tasks.empty()) {
			auto [s, p] = tasks.back();
			tasks.pop_back();
			work.clear(), work.push_back(s), previous[s] = -2;
			for (std::size_t i = 0; i < work.size(); ++i) {
				int v = work[i];
				sub[v] = 1;
				for (int j = start[v]; j < start[v + 1]; ++j) {
					int u = adj[j];
					if (depth[u] >= 0 || u == previous[v]) continue;
					assert(p != -1 || previous[u] == -1);
					previous[u] = v, work.push_back(u);
				}
			}
			assert(p != -1 || work.size() == std::size_t(n));
			int count = int(work.size()), c = -1;
			for (std::size_t i = work.size(); i-- > 0;) {
				int v = work[i];
				if (c == -1 && sub[v] >= (count + 1) / 2) c = v;
				if (previous[v] >= 0) sub[previous[v]] += sub[v];
			}
			parent[c] = p, depth[c] = p == -1 ? 0 : depth[p] + 1;
			size[c] = count, in[c] = int(order.size()), out[c] = in[c] + count;
			order.push_back(c);
			if (p == -1) root = c;
			for (int j = start[c + 1]; j-- > start[c];) {
				int u = adj[j];
				if (depth[u] == -1) tasks.emplace_back(u, c);
			}
		}
		if (distances) {
			offsets_.resize(n + 1), offsets_[0] = 0;
			for (int v = 0; v < n; ++v) offsets_[v + 1] = offsets_[v] + depth[v] + 1;
			paths_.resize(offsets_.back());
			std::vector<T> distance(n);
			for (int c : order) {
				work.clear(), work.push_back(c), previous[c] = -1, distance[c] = T(0);
				for (std::size_t i = 0; i < work.size(); ++i) {
					int v = work[i];
					paths_[offsets_[v] + depth[c]] = {c, -1, distance[v]};
					for (int j = start[v]; j < start[v + 1]; ++j) {
						int u = adj[j];
						if (u == previous[v] || depth[u] < depth[c]) continue;
						previous[u] = v, distance[u] = distance[v] + weights[j];
						work.push_back(u);
					}
				}
			}
			for (int v = 0; v < n; ++v)
				for (std::size_t i = offsets_[v]; i + 1 < offsets_[v + 1]; ++i)
					paths_[i].branch = paths_[i + 1].centroid;
		}
		built_ = true, distances_ = distances;
	}
	bool has_distances() const { return built_ && distances_; }
	std::span<const Ancestor> ancestors(int v) const {
		check(v);
		assert(distances_);
		return std::span<const Ancestor>(paths_).subspan(offsets_[v], offsets_[v + 1] - offsets_[v]);
	}
	template<class F> void for_each_ancestor(int v, F&& f) const {
		auto path = ancestors(v);
		for (std::size_t i = path.size(); i-- > 0;) {
			const auto& a = path[i];
			f(a.centroid, a.distance, a.branch);
		}
	}
	bool is_ancestor(int c, int v) const {
		check(c), check(v);
		return in[c] <= in[v] && in[v] < out[c];
	}
	std::pair<int, int> subtree(int c) const {
		check(c);
		return {in[c], out[c]};
	}
	int centroid_lca(int u, int v) const {
		check(u), check(v);
		while (depth[u] > depth[v]) u = parent[u];
		while (depth[v] > depth[u]) v = parent[v];
		while (u != v) u = parent[u], v = parent[v];
		return u;
	}
	T distance_to_ancestor(int v, int c) const {
		assert(distances_ && is_ancestor(c, v));
		check(v), check(c);
		return paths_[offsets_[v] + depth[c]].distance;
	}
	T dist(int u, int v) const {
		int c = centroid_lca(u, v);
		return distance_to_ancestor(u, c) + distance_to_ancestor(v, c);
	}
};

#endif
