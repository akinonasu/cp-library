#ifndef RANGE_EDGE_GRAPH_HPP
#define RANGE_EDGE_GRAPH_HPP

#include "../components/directed_csr.hpp"
#include <limits>

template<class Cost = std::int64_t>
class RangeEdgeGraph {
	int n_, vertices_;
	std::vector<typename DirectedCSR<Cost>::Edge> edges_;
	int out_(int k) const { return k >= n_ ? k - n_ : n_ + k - 1; }
	int in_(int k) const { return k >= n_ ? k - n_ : 2 * n_ + k - 2; }
	void vertex_([[maybe_unused]] int v) const { assert(0 <= v && v < n_); }
	void range_([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }
	template<class Emit> void cover_(int l, int r, Emit emit) const {
		if (l == r) return;
		if (l == 0 && r == n_) { emit(1); return; }
		for (l += n_, r += n_; l < r; l >>= 1, r >>= 1) {
			if (l & 1) emit(l++);
			if (r & 1) emit(--r);
		}
	}

public:
	using weight_type = Cost;
	using Edge = typename DirectedCSR<Cost>::Edge;
	explicit RangeEdgeGraph(int n = 0) : n_(n), vertices_(0) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 3);
		vertices_ = n ? 3 * n - 2 : 0;
		edges_.reserve(n ? std::size_t(n - 1) * 4 : 0);
		for (int k = 1; k < n_; ++k) for (int child : {2 * k, 2 * k + 1}) {
			edges_.push_back({out_(k), out_(child), Cost(0)});
			edges_.push_back({in_(child), in_(k), Cost(0)});
		}
	}
	int size() const { return n_; }
	int vertex_count() const { return vertices_; }
	std::size_t edge_count() const { return edges_.size(); }
	void reserve_edges(std::size_t m) { edges_.reserve(m); }
	void add_edge(int from, int to, Cost weight = Cost(1)) {
		vertex_(from), vertex_(to);
		edges_.push_back({from, to, std::move(weight)});
	}
	void add_to_range(int from, int l, int r, Cost weight = Cost(1)) {
		vertex_(from), range_(l, r);
		cover_(l, r, [&](int k) { edges_.push_back({from, out_(k), weight}); });
	}
	void add_from_range(int l, int r, int to, Cost weight = Cost(1)) {
		range_(l, r), vertex_(to);
		cover_(l, r, [&](int k) { edges_.push_back({in_(k), to, weight}); });
	}
	void add_range_to_range(int l1, int r1, int l2, int r2, Cost weight = Cost(1)) {
		range_(l1, r1), range_(l2, r2);
		if (l1 == r1 || l2 == r2) return;
		if (r1 - l1 == 1) { add_to_range(l1, l2, r2, std::move(weight)); return; }
		if (r2 - l2 == 1) { add_from_range(l1, r1, l2, std::move(weight)); return; }
		assert(vertices_ < std::numeric_limits<int>::max());
		int via = vertices_++;
		cover_(l1, r1, [&](int k) { edges_.push_back({in_(k), via, weight}); });
		cover_(l2, r2, [&](int k) { edges_.push_back({via, out_(k), Cost(0)}); });
	}
	const std::vector<Edge> &edges() const { return edges_; }
	DirectedCSR<Cost> build() const { return DirectedCSR<Cost>(vertices_, edges_); }
	std::vector<std::vector<std::pair<int, Cost>>> adjacency_list() const {
		std::vector<std::vector<std::pair<int, Cost>>> graph(vertices_);
		std::vector<std::size_t> degrees(vertices_);
		for (const auto &edge : edges_) ++degrees[edge.from];
		for (int v = 0; v < vertices_; ++v) graph[v].reserve(degrees[v]);
		for (const auto &edge : edges_) graph[edge.from].emplace_back(edge.to, edge.weight);
		return graph;
	}
	std::vector<int> original_path(const std::vector<int> &path) const {
		std::vector<int> result;
		for (int v : path) {
			assert(0 <= v && v < vertices_);
			if (v < n_) result.push_back(v);
		}
		return result;
	}
};

#endif
