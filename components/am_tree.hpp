#ifndef AM_TREE_HPP
#define AM_TREE_HPP

#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace am_tree_detail {
template<class Key, auto less> class Tree {
	struct Node { int parent = 0, size = 1; Key key{}; };
	std::vector<Node> nodes_;
	int components_ = 0;
	static std::size_t count(int n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
		return std::size_t(n) + 1;
	}
	void promote(int x) {
		auto& a = nodes_[x];
		int y = a.parent;
		auto& b = nodes_[y];
		b.size -= a.size;
		a.parent = b.parent;
		if (!b.parent || less(a.key, b.key)) {
			a.size += b.size;
			b.parent = x;
			std::swap(a.key, b.key);
		}
	}
	void calibrate(int x) {
		while (int p = nodes_[x].parent) {
			if (std::int64_t(nodes_[x].size) * 3 <= std::int64_t(nodes_[p].size) * 2) x = p;
			else promote(x);
		}
	}
	void perch(int x, int stop) {
		while (nodes_[x].parent && nodes_[x].parent != stop) promote(x);
	}
	int maximum(int x, int y) const {
		int best = 0;
		while (x != y) {
			if (nodes_[x].size > nodes_[y].size) std::swap(x, y);
			if (!nodes_[x].parent) return 0;
			if (!best || less(nodes_[best].key, nodes_[x].key)) best = x;
			x = nodes_[x].parent;
		}
		return best;
	}

  public:
	explicit Tree(int n = 0) : nodes_(count(n)), components_(n) { nodes_[0].size = 0; }
	Tree(const Tree&) = default;
	Tree& operator=(const Tree&) = default;
	Tree(Tree&& other) noexcept : nodes_(std::move(other.nodes_)), components_(std::exchange(other.components_, 0)) { other.nodes_.clear(); }
	Tree& operator=(Tree&& other) noexcept {
		if (this != &other) {
			nodes_ = std::move(other.nodes_);
			components_ = std::exchange(other.components_, 0);
			other.nodes_.clear();
		}
		return *this;
	}
	int size() const { return nodes_.empty() ? 0 : int(nodes_.size()) - 1; }
	int component_count() const { return components_; }
	void clear() {
		int n = size();
		nodes_.assign(std::size_t(n) + 1, Node{});
		nodes_[0].size = 0;
		components_ = n;
	}
	std::optional<Key> insert(int u, int v, Key key) {
		++u, ++v;
		calibrate(u), calibrate(v);
		perch(v, 0), perch(u, v);
		if (nodes_[u].parent == v) {
			if (!less(key, nodes_[u].key)) return key;
			return std::exchange(nodes_[u].key, key);
		}
		assert(!nodes_[u].parent && !nodes_[v].parent);
		if (nodes_[u].size > nodes_[v].size) std::swap(u, v);
		nodes_[u].parent = v;
		nodes_[u].key = key;
		nodes_[v].size += nodes_[u].size;
		--components_;
		return std::nullopt;
	}
	std::optional<Key> path_max(int u, int v) {
		if (u == v) return std::nullopt;
		++u, ++v;
		calibrate(u), calibrate(v);
		int best = maximum(u, v);
		if (!best) return std::nullopt;
		return nodes_[best].key;
	}
	int component_size(int v) {
		++v;
		calibrate(v);
		while (nodes_[v].parent) v = nodes_[v].parent;
		return nodes_[v].size;
	}
	template<class F> void for_each(F f) const {
		for (const auto& node : nodes_) if (node.parent) f(node.key);
	}
};
}

#endif
