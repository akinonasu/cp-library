#ifndef TREE_DIAMETER_HPP
#define TREE_DIAMETER_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template<class T = std::int64_t> class TreeDiameter {
	int n_;

  public:
	struct Edge { int from, to; T weight; };
	struct Result { T distance{}; std::vector<int> path; };

  private:
	std::vector<Edge> edges_;

  public:
	explicit TreeDiameter(int n = 0) : n_(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		edges_.reserve(n ? n - 1 : 0);
	}
	TreeDiameter(const TreeDiameter &) = default;
	TreeDiameter &operator=(const TreeDiameter &other) {
		if (this != &other) { TreeDiameter copy(other); swap(copy); }
		return *this;
	}
	TreeDiameter(TreeDiameter &&other) noexcept : n_(std::exchange(other.n_, 0)), edges_(std::move(other.edges_)) {}
	TreeDiameter &operator=(TreeDiameter &&other) noexcept {
		if (this != &other) n_ = std::exchange(other.n_, 0), edges_ = std::move(other.edges_);
		return *this;
	}
	void swap(TreeDiameter &other) noexcept {
		std::swap(n_, other.n_);
		edges_.swap(other.edges_);
	}
	int size() const { return n_; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(n_ ? n_ - 1 : 0));
		edges_.reserve(m);
	}
	int add_edge(int u, int v, T weight = T(1)) {
		assert(0 <= u && u < n_ && 0 <= v && v < n_ && u != v);
		assert(edges_.size() < std::size_t(n_ - 1));
		int id = edge_count();
		edges_.push_back({u, v, std::move(weight)});
		return id;
	}
	const std::vector<Edge> &edges() const { return edges_; }
	void clear() { edges_.clear(); }
	Result solve() const {
		assert(edges_.size() == std::size_t(n_ ? n_ - 1 : 0));
		Result result;
		if (!n_) return result;
		struct State { int degree = 0, link = 0, parent = -1, endpoint = 0; T best{}; };
		std::vector<State> state(n_);
		for (int id = 0; id < edge_count(); ++id) {
			const auto &e = edges_[id];
			++state[e.from].degree, ++state[e.to].degree;
			state[e.from].link ^= id, state[e.to].link ^= id;
		}
		std::vector<int> order;
		order.reserve(n_ - 1);
		for (int v = 0; v < n_; ++v) {
			state[v].endpoint = v;
			if (v && state[v].degree == 1) order.push_back(v);
		}
		int from = 0, to = 0, middle = 0;
		for (std::size_t i = 0; i < order.size(); ++i) {
			int v = order[i], id = state[v].link;
			const auto &e = edges_[id];
			int p = e.from ^ e.to ^ v;
			state[v].parent = p;
			T candidate = state[v].best + e.weight;
			T joined = state[p].best + candidate;
			if (result.distance < joined) {
				result.distance = std::move(joined);
				from = state[v].endpoint, to = state[p].endpoint, middle = p;
			}
			if (state[p].best < candidate) state[p].best = std::move(candidate), state[p].endpoint = state[v].endpoint;
			state[p].link ^= id;
			if (--state[p].degree == 1 && p) order.push_back(p);
		}
		assert(order.size() == std::size_t(n_ - 1));
		for (int v = from; v != middle; v = state[v].parent) result.path.push_back(v);
		result.path.push_back(middle);
		std::size_t first = result.path.size();
		for (int v = to; v != middle; v = state[v].parent) result.path.push_back(v);
		std::reverse(result.path.begin() + first, result.path.end());
		return result;
	}
};

#endif
