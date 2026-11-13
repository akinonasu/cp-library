#ifndef EDGE_LINK_CUT_TREE_HPP
#define EDGE_LINK_CUT_TREE_HPP

#include "../components/link_cut_tree_core.hpp"
#include <cassert>
#include <limits>
#include <map>
#include <optional>
#include <utility>
#include <vector>

template<class S, auto op, auto e>
class EdgeLinkCutTree {
	struct Record { int slot, from, to; };
	int n_;
	link_cut_tree_detail::Tree<S, op, e, false, true> tree_;
	std::vector<int> next_;
	std::map<std::pair<int, int>, Record> edges_;
	int free_;
	static int node_count(int n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		return n ? 2 * n - 1 : 0;
	}
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < n_); }
	static std::pair<int, int> key(int u, int v) { return u < v ? std::pair{u, v} : std::pair{v, u}; }
	int node(const Record &edge) const { return n_ + edge.slot; }

  public:
	struct Edge { int from, to; S forward, backward; };
	explicit EdgeLinkCutTree(int n = 0) : n_(n), tree_(node_count(n)), next_(n ? n - 1 : 0), free_(n > 1 ? 0 : -1) {
		for (int i = 0; i < n - 1; ++i) next_[i] = i + 1 < n - 1 ? i + 1 : -1;
	}
	EdgeLinkCutTree(const EdgeLinkCutTree &) = default;
	EdgeLinkCutTree &operator=(const EdgeLinkCutTree &) = default;
	EdgeLinkCutTree(EdgeLinkCutTree &&other) noexcept
		: n_(std::exchange(other.n_, 0)), tree_(std::move(other.tree_)), next_(std::move(other.next_)),
		  edges_(std::move(other.edges_)), free_(std::exchange(other.free_, -1)) {}
	EdgeLinkCutTree &operator=(EdgeLinkCutTree &&other) noexcept {
		if (this != &other) {
			n_ = std::exchange(other.n_, 0);
			tree_ = std::move(other.tree_);
			next_ = std::move(other.next_);
			edges_ = std::move(other.edges_);
			free_ = std::exchange(other.free_, -1);
		}
		return *this;
	}
	int size() const { return n_; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	bool same(int u, int v) { vertex(u), vertex(v); return tree_.same(u, v); }
	bool link(int u, int v, S value) { return link(u, v, value, value); }
	bool link(int u, int v, S forward, S backward) {
		vertex(u), vertex(v);
		if (tree_.same(u, v)) return false;
		assert(free_ >= 0);
		Record edge{free_, u, v};
		[[maybe_unused]] bool inserted = edges_.emplace(key(u, v), edge).second;
		assert(inserted);
		free_ = next_[free_];
		[[maybe_unused]] bool first = tree_.link(u, node(edge)), second = tree_.link(node(edge), v);
		assert(first && second);
		tree_.make_root(u);
		tree_.set(node(edge), std::move(forward), std::move(backward));
		return true;
	}
	bool cut(int u, int v) {
		vertex(u), vertex(v);
		auto it = edges_.find(key(u, v));
		if (it == edges_.end()) return false;
		Record edge = it->second;
		[[maybe_unused]] bool first = tree_.cut(edge.from, node(edge)), second = tree_.cut(node(edge), edge.to);
		assert(first && second);
		next_[edge.slot] = free_;
		free_ = edge.slot;
		edges_.erase(it);
		return true;
	}
	std::optional<Edge> get(int u, int v) {
		vertex(u), vertex(v);
		auto it = edges_.find(key(u, v));
		if (it == edges_.end()) return std::nullopt;
		const auto &edge = it->second;
		tree_.make_root(edge.from);
		auto [forward, backward] = tree_.get(node(edge));
		return Edge{edge.from, edge.to, std::move(forward), std::move(backward)};
	}
	bool set(int u, int v, S value) { return set(u, v, value, value); }
	bool set(int u, int v, S forward, S backward) {
		vertex(u), vertex(v);
		auto it = edges_.find(key(u, v));
		if (it == edges_.end()) return false;
		const auto &edge = it->second;
		tree_.make_root(edge.from);
		tree_.set(node(edge), std::move(forward), std::move(backward));
		return true;
	}
	S prod(int u, int v) { vertex(u), vertex(v); return tree_.prod(u, v); }
	int distance(int u, int v) { vertex(u), vertex(v); return tree_.distance(u, v) / 2; }
	int path_size(int u, int v) { return distance(u, v) + 1; }
	void make_root(int v) { vertex(v); tree_.make_root(v); }
	int root(int v) {
		vertex(v);
		int result = tree_.root(v);
		assert(0 <= result && result < n_);
		return result;
	}
	int jump(int u, int v, int k) {
		vertex(u), vertex(v);
		if (k < 0 || k >= n_) return -1;
		int result = tree_.jump(u, v, 2 * k);
		assert(result < n_);
		return result;
	}
	int lca(int r, int u, int v) {
		vertex(r), vertex(u), vertex(v);
		int result = tree_.lca(r, u, v);
		assert(0 <= result && result < n_);
		return result;
	}
};

#endif
