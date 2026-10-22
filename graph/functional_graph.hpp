#ifndef FUNCTIONAL_GRAPH_HPP
#define FUNCTIONAL_GRAPH_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

class FunctionalGraph {
	std::vector<int> head_, in_, order_, cycle_vertices_, cycle_offsets_;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }

  public:
	std::vector<int> to, parent, cycle_id, cycle_position, entry, depth, subtree_size;
	FunctionalGraph() = default;
	explicit FunctionalGraph(std::vector<int> next) { build(std::move(next)); }
	void build(std::vector<int> next) {
		assert(next.size() <= std::size_t(std::numeric_limits<int>::max()));
		const int n = static_cast<int>(next.size());
		for ([[maybe_unused]] int v : next) assert(0 <= v && v < n);
		to = std::move(next);
		parent.resize(n);
		cycle_id.assign(n, -1);
		cycle_position.assign(n, -1);
		entry.resize(n);
		depth.assign(n, 0);
		subtree_size.assign(n, 1);
		head_.resize(n);
		in_.resize(n);
		order_.resize(n);
		cycle_vertices_.clear();
		cycle_offsets_.clear();
		std::vector<int> indegree(n), heavy(n, -1), peeled;
		peeled.reserve(n);
		for (int v : to) ++indegree[v];
		for (int v = 0; v < n; ++v) if (!indegree[v]) peeled.push_back(v);
		for (std::size_t i = 0; i < peeled.size(); ++i) {
			int v = peeled[i], p = to[v];
			parent[v] = p;
			subtree_size[p] += subtree_size[v];
			if (heavy[p] == -1 || subtree_size[heavy[p]] < subtree_size[v]) heavy[p] = v;
			if (!--indegree[p]) peeled.push_back(p);
		}
		cycle_vertices_.reserve(std::size_t(n) - peeled.size());
		int position = 0;
		for (int v = 0; v < n; ++v) if (indegree[v] && cycle_id[v] == -1) {
			int id = cycle_count(), start = static_cast<int>(cycle_vertices_.size()), u = v;
			cycle_offsets_.push_back(start);
			do {
				cycle_id[u] = id;
				cycle_position[u] = static_cast<int>(cycle_vertices_.size()) - start;
				cycle_vertices_.push_back(u);
				entry[u] = u;
				parent[u] = -1;
				head_[u] = u;
				in_[u] = position;
				position += subtree_size[u];
				order_[u] = position;
				u = to[u];
			} while (u != v);
		}
		assert(position == n);
		for (auto it = peeled.rbegin(); it != peeled.rend(); ++it) {
			int v = *it, p = to[v];
			cycle_id[v] = cycle_id[p];
			entry[v] = entry[p];
			depth[v] = depth[p] + 1;
			if (heavy[p] == v) in_[v] = in_[p] + 1, head_[v] = head_[p];
			else in_[v] = order_[p] -= subtree_size[v], head_[v] = v;
			order_[v] = in_[v] + subtree_size[v];
		}
		for (int v = 0; v < n; ++v) order_[in_[v]] = v;
	}
	int size() const { return static_cast<int>(to.size()); }
	bool empty() const { return to.empty(); }
	int cycle_count() const { return static_cast<int>(cycle_offsets_.size()); }
	std::span<const int> cycle(int id) const {
		assert(0 <= id && id < cycle_count());
		int start = cycle_offsets_[id];
		int end = id + 1 < cycle_count() ? cycle_offsets_[id + 1] : static_cast<int>(cycle_vertices_.size());
		return std::span<const int>(cycle_vertices_).subspan(start, end - start);
	}
	int cycle_length(int v) const {
		check(v);
		return static_cast<int>(cycle(cycle_id[v]).size());
	}
	bool on_cycle(int v) const {
		check(v);
		return depth[v] == 0;
	}
	bool same_component(int u, int v) const {
		check(u), check(v);
		return cycle_id[u] == cycle_id[v];
	}
	int reachable_count(int v) const {
		check(v);
		return depth[v] + cycle_length(v);
	}
	int jump(int v, std::uint64_t k) const {
		check(v);
		if (k < std::uint64_t(depth[v])) {
			int target = depth[v] - static_cast<int>(k);
			while (depth[head_[v]] > target) v = to[head_[v]];
			return order_[in_[v] - (depth[v] - target)];
		}
		k -= depth[v];
		v = entry[v];
		auto vertices = cycle(cycle_id[v]);
		std::size_t position = cycle_position[v] + k % vertices.size();
		if (position >= vertices.size()) position -= vertices.size();
		return vertices[position];
	}
	int dist(int u, int v) const {
		if (!same_component(u, v)) return -1;
		if (depth[v]) return in_[v] <= in_[u] && in_[u] < in_[v] + subtree_size[v] ? depth[u] - depth[v] : -1;
		int offset = cycle_position[v] - cycle_position[entry[u]];
		if (offset < 0) offset += cycle_length(v);
		return depth[u] + offset;
	}
	int lca(int u, int v) const {
		check(u), check(v);
		if (entry[u] != entry[v]) return -1;
		while (head_[u] != head_[v]) {
			if (depth[head_[u]] < depth[head_[v]]) std::swap(u, v);
			u = to[head_[u]];
		}
		return depth[u] < depth[v] ? u : v;
	}
	std::pair<int, int> subtree(int v) const {
		check(v);
		return {in_[v], in_[v] + subtree_size[v]};
	}
	int vertex_at(int position) const {
		assert(0 <= position && position < size());
		return order_[position];
	}
	std::vector<std::vector<int>> forest() const {
		std::vector<std::vector<int>> result(size());
		for (int v = 0; v < size(); ++v) if (parent[v] != -1) {
			result[v].push_back(parent[v]);
			result[parent[v]].push_back(v);
		}
		return result;
	}
};

#endif
