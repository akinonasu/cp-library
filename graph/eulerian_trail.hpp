#ifndef EULERIAN_TRAIL_HPP
#define EULERIAN_TRAIL_HPP

#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <utility>
#include <vector>

template<bool Directed> class EulerianTrail {
	struct Arc { int to, id; };
	int n_;
	std::vector<std::pair<int, int>> edges_;
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < n_); }

  public:
	struct Result { std::vector<int> vertices, edges; };
	explicit EulerianTrail(int n = 0) : n_(n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
	}
	EulerianTrail(int n, std::span<const std::pair<int, int>> edges) : EulerianTrail(n) {
		reserve(edges.size());
		for (auto [u, v] : edges) add_edge(u, v);
	}
	int size() const { return n_; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(std::numeric_limits<int>::max() / 2));
		edges_.reserve(m);
	}
	int add_edge(int u, int v) {
		vertex(u), vertex(v);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		int id = edge_count();
		edges_.emplace_back(u, v);
		return id;
	}
	std::pair<int, int> get_edge(int id) const {
		assert(0 <= id && id < edge_count());
		return edges_[id];
	}
	const std::vector<std::pair<int, int>> &edges() const { return edges_; }
	void clear() { edges_.clear(); }
	std::optional<Result> solve(int start = -1) const {
		assert(start == -1 || (0 <= start && start < n_));
		const int m = edge_count();
		if (!m) {
			Result result;
			if (n_) result.vertices.push_back(start == -1 ? 0 : start);
			return result;
		}
		std::vector<int> offset(std::size_t(n_) + 1), balance;
		if constexpr (Directed) balance.resize(n_);
		for (auto [u, v] : edges_) {
			++offset[u + 1];
			if constexpr (Directed) ++balance[u], --balance[v];
			else ++offset[v + 1];
		}
		int required = -1, finish = -1, active = -1;
		for (int v = 0; v < n_; ++v) {
			if (active == -1 && offset[v + 1]) active = v;
			if constexpr (Directed) {
				if (balance[v] == 1 && required == -1) required = v;
				else if (balance[v] == -1 && finish == -1) finish = v;
				else if (balance[v] != 0) return std::nullopt;
			} else if (offset[v + 1] & 1) {
				if (required == -1) required = v;
				else if (finish == -1) finish = v;
				else return std::nullopt;
			}
		}
		if ((required == -1) != (finish == -1)) return std::nullopt;
		if (start == -1) start = required == -1 ? active : required;
		if (required != -1 && start != required) {
			if constexpr (Directed) return std::nullopt;
			else if (start != finish) return std::nullopt;
		}
		if (!offset[start + 1]) return std::nullopt;
		for (int v = 0; v < n_; ++v) offset[v + 1] += offset[v];
		std::unique_ptr<Arc[]> adjacency(new Arc[offset.back()]);
		std::vector<int> cursor(offset.begin(), offset.end() - 1);
		for (int id = 0; id < m; ++id) {
			auto [u, v] = edges_[id];
			adjacency[cursor[u]++] = {v, id};
			if constexpr (!Directed) adjacency[cursor[v]++] = {u, id};
		}
		std::vector<unsigned char> used;
		if constexpr (!Directed) used.resize(m);
		Result result{std::vector<int>(std::size_t(m) + 1), std::vector<int>(m)};
		result.vertices[0] = start;
		int top = 1, vertex_out = m + 1, edge_out = m;
		while (top) {
			int v = result.vertices[top - 1];
			if constexpr (!Directed)
				while (cursor[v] > offset[v] && used[adjacency[cursor[v] - 1].id]) --cursor[v];
			if (cursor[v] > offset[v]) {
				auto [to, id] = adjacency[--cursor[v]];
				if constexpr (!Directed) used[id] = 1;
				result.edges[top - 1] = id;
				result.vertices[top++] = to;
			} else {
				int edge = top > 1 ? result.edges[top - 2] : -1;
				--top;
				result.vertices[--vertex_out] = v;
				if (edge != -1) result.edges[--edge_out] = edge;
			}
		}
		if (edge_out) return std::nullopt;
		return result;
	}
};

using DirectedEulerianTrail = EulerianTrail<true>;
using UndirectedEulerianTrail = EulerianTrail<false>;

#endif
