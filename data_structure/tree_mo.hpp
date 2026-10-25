#ifndef TREE_MO_HPP
#define TREE_MO_HPP

#include "mo_order.hpp"
#include "../tree/lca.hpp"

class TreeMo {
	LCA lca_;
	std::vector<int> enter_, leave_, tour_, extra_;
	std::vector<std::pair<int, int>> ranges_;
	bool edge_values_;

public:
	explicit TreeMo(const std::vector<std::vector<int>> &graph, int root = 0, bool edge_values = false)
		: lca_(), edge_values_(edge_values) {
		assert(graph.size() <= std::size_t(std::numeric_limits<int>::max() / 2));
		int n = static_cast<int>(graph.size());
		enter_.resize(n), leave_.resize(n);
		if (!n) return;
		assert(0 <= root && root < n);
		std::size_t edges = 0;
		for (const auto &row : graph) edges += row.size();
		assert(edges == std::size_t(n - 1) * 2);
		std::vector<int> parents(n, -2), stack{root};
		std::vector<std::size_t> next(n);
		parents[root] = -1;
		tour_.reserve(std::size_t(n) * 2);
		tour_.push_back(root);
		while (!stack.empty()) {
			int v = stack.back();
			if (next[v] < graph[v].size()) {
				int to = graph[v][next[v]++];
				assert(0 <= to && to < n);
				if (to == parents[v]) continue;
				assert(parents[to] == -2);
				parents[to] = v;
				enter_[to] = static_cast<int>(tour_.size());
				tour_.push_back(to);
				stack.push_back(to);
			} else {
				leave_[v] = static_cast<int>(tour_.size());
				tour_.push_back(v);
				stack.pop_back();
			}
		}
		assert(tour_.size() == std::size_t(n) * 2);
		lca_.build_from_parents(std::move(parents));
	}
	int size() const { return lca_.n; }
	int query_count() const { return static_cast<int>(ranges_.size()); }
	int parent(int v) const {
		assert(0 <= v && v < size());
		return lca_.parent[v];
	}
	int add_query(int u, int v) {
		assert(0 <= u && u < size() && 0 <= v && v < size());
		assert(ranges_.size() < std::size_t(std::numeric_limits<int>::max()));
		if (enter_[u] > enter_[v]) std::swap(u, v);
		int w = lca_.lca(u, v);
		if (w == u) {
			ranges_.emplace_back(enter_[u] + int(edge_values_), enter_[v] + 1);
			extra_.push_back(-1);
		} else {
			ranges_.emplace_back(leave_[u], enter_[v] + 1);
			extra_.push_back(edge_values_ ? -1 : w);
		}
		return query_count() - 1;
	}
	std::vector<int> order(int block_size = 0) const { return mo_order(ranges_, block_size); }
	template<class Add, class Erase, class Answer>
	void run(Add add, Erase erase, Answer answer, int block_size = 0) const {
		if (!query_count()) { assert(block_size >= 0); return; }
		std::vector<unsigned char> active(size());
		auto toggle = [&](int v) {
			if (active[v]) erase(v);
			else add(v);
			active[v] ^= 1;
		};
		int l = 0, r = 0;
		for (int id : order(block_size)) {
			auto [a, b] = ranges_[id];
			while (a < l) toggle(tour_[--l]);
			while (r < b) toggle(tour_[r++]);
			while (l < a) toggle(tour_[l++]);
			while (b < r) toggle(tour_[--r]);
			if (extra_[id] >= 0) toggle(extra_[id]);
			answer(id);
			if (extra_[id] >= 0) toggle(extra_[id]);
		}
	}
};

#endif
