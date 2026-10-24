#ifndef PERMUTATION_TREE_HPP
#define PERMUTATION_TREE_HPP

#include "../components/static_range_min.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

class PermutationTree {
  public:
	enum class Type : unsigned char { Leaf, Increasing, Decreasing, Prime };
	struct Node {
		int parent, l, r, lo, hi;
		Type type;
	};

  private:
	int root_ = -1;
	std::int64_t count_ = 0;
	std::vector<Node> nodes_;
	std::vector<int> offsets_, children_, order_;
	int add_node(int l, int r, int lo, int hi, Type type) {
		int id = size();
		nodes_.push_back({-1, l, r, lo, hi, type});
		return id;
	}
	void pack(int n) {
		int m = size();
		offsets_.assign(m + 1, 0);
		for (auto &node : nodes_) if (node.parent >= 0) ++offsets_[node.parent + 1];
		for (int i = 0; i < m; ++i) offsets_[i + 1] += offsets_[i];
		children_.resize(m - 1);
		std::vector<int> cursor = offsets_;
		for (int i = 0; i < n; ++i) {
			int v = i;
			while (nodes_[v].parent >= 0) {
				int parent = nodes_[v].parent;
				children_[cursor[parent]++] = v;
				if (nodes_[parent].l != i) break;
				v = parent;
			}
		}
		count_ = n;
		for (int i = n; i < m; ++i) {
			std::int64_t k = offsets_[i + 1] - offsets_[i];
			count_ += nodes_[i].type == Type::Prime ? 1 : k * (k - 1) / 2;
		}
		order_.clear();
		order_.reserve(m);
		order_.push_back(root_);
		for (int i = 0; i < m; ++i)
			for (int child : children(order_[i])) order_.push_back(child);
	}

  public:
	PermutationTree() = default;
	explicit PermutationTree(std::span<const int> permutation) { build(permutation); }
	void build(std::span<const int> permutation) {
		assert(permutation.size() <= std::size_t(std::numeric_limits<int>::max() / 2));
		int n = static_cast<int>(permutation.size());
		root_ = -1;
		count_ = 0;
		nodes_.clear();
		offsets_.clear();
		children_.clear();
		order_.clear();
		if (!n) return;
		nodes_.reserve(2 * std::size_t(n) - 1);
		std::vector<int> inverse(n, -1);
		for (int i = 0; i < n; ++i) {
			int x = permutation[i];
			assert(0 <= x && x < n && inverse[x] < 0);
			inverse[x] = i;
			add_node(i, i + 1, x, x + 1, Type::Leaf);
		}
		struct Frame { int l, lo, hi; };
		struct Interval { int l, r, lo, hi; };
		std::vector<Interval> intervals;
		{
			StaticRangeMin minimum(std::move(inverse));
			std::vector<Frame> stack;
			stack.reserve(n);
			intervals.reserve(n - 1);
			for (int r = 1; r <= n; ++r) {
				int x = permutation[r - 1];
				Frame current{r - 1, x, x + 1};
				while (!stack.empty()) {
					auto &top = stack.back();
					top.lo = std::min(top.lo, current.lo);
					top.hi = std::max(top.hi, current.hi);
					if (top.hi - top.lo == r - top.l) {
						current = top;
						intervals.push_back({top.l, r, top.lo, top.hi});
						stack.pop_back();
					} else if (minimum.prod(top.lo, top.hi) < top.l) {
						Frame removed = top;
						stack.pop_back();
						assert(!stack.empty());
						stack.back().lo = std::min(stack.back().lo, removed.lo);
						stack.back().hi = std::max(stack.back().hi, removed.hi);
					} else break;
				}
				stack.push_back(current);
			}
			assert(stack.size() == 1 && stack[0].l == 0);
		}
		std::vector<int> at(n), next(n);
		for (int i = 0; i < n; ++i) at[i] = i, next[i] = i + 1;
		for (auto interval : intervals) {
			int l = interval.l, r = interval.r, middle = next[l];
			int first = at[l], id;
			if (next[middle] == r) {
				int second = at[middle];
				Type type = nodes_[first].hi == nodes_[second].lo ? Type::Increasing : Type::Decreasing;
				if (nodes_[first].type == type) {
					id = first;
					nodes_[id].r = r;
					nodes_[id].lo = interval.lo;
					nodes_[id].hi = interval.hi;
					nodes_[second].parent = id;
				} else {
					id = add_node(l, r, interval.lo, interval.hi, type);
					nodes_[first].parent = nodes_[second].parent = id;
				}
			} else {
				id = add_node(l, r, interval.lo, interval.hi, Type::Prime);
				for (int i = l; i < r; i = next[i]) nodes_[at[i]].parent = id;
			}
			at[l] = id;
			next[l] = r;
		}
		root_ = at[0];
		assert(next[0] == n);
		pack(n);
	}
	int size() const { return static_cast<int>(nodes_.size()); }
	bool empty() const { return nodes_.empty(); }
	int root() const { return empty() ? -1 : root_; }
	int length() const { return empty() ? 0 : nodes_[root_].r; }
	int leaf(int i) const { assert(0 <= i && i < length()); return i; }
	const Node &operator[](int i) const {
		assert(0 <= i && i < size());
		return nodes_[i];
	}
	const std::vector<Node> &nodes() const { return nodes_; }
	std::span<const int> children(int v) const {
		assert(0 <= v && v < size());
		return std::span<const int>(children_).subspan(offsets_[v], offsets_[v + 1] - offsets_[v]);
	}
	std::span<const int> order() const { return order_; }
	std::int64_t count_intervals() const { return empty() ? 0 : count_; }
};

#endif
