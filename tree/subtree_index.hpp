#ifndef SUBTREE_INDEX_HPP
#define SUBTREE_INDEX_HPP

#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

class SubtreeIndex {
	int root_ = -1;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
public:
	std::vector<int> in, out, order;
	SubtreeIndex() = default;
	explicit SubtreeIndex(std::vector<int> parents) { build_from_parents(std::move(parents)); }
	SubtreeIndex(const SubtreeIndex &) = default;
	SubtreeIndex(SubtreeIndex &&other) noexcept { swap(other); }
	SubtreeIndex &operator=(SubtreeIndex other) noexcept { swap(other); return *this; }
	void swap(SubtreeIndex &other) noexcept {
		std::swap(root_, other.root_);
		in.swap(other.in), out.swap(other.out), order.swap(other.order);
	}
	friend void swap(SubtreeIndex &a, SubtreeIndex &b) noexcept { a.swap(b); }
	int size() const { return int(in.size()); }
	bool empty() const { return in.empty(); }
	int root() const { return root_; }
	void clear() { root_ = -1; in.clear(); out.clear(); order.clear(); }
	void build_from_parents(std::vector<int> parents) {
		assert(parents.size() <= std::size_t(std::numeric_limits<int>::max()));
		int n = int(parents.size());
		root_ = -1;
		in.resize(n), out.assign(n, 1), order.resize(n);
		if (!n) return;
		bool ordered = true;
		for (int v = 0; v < n; ++v) {
			int p = parents[v];
			assert(-1 <= p && p < n && p != v);
			if (p == -1) { assert(root_ == -1); root_ = v; }
			else if (p >= v) ordered = false;
		}
		assert(root_ != -1);
		auto finish = [&](auto vertex) {
			for (int i = n - 1; i > 0; --i) {
				int v = vertex(i);
				out[parents[v]] += out[v];
			}
			in[root_] = 0;
			order[root_] = 1;
			for (int i = 1; i < n; ++i) {
				int v = vertex(i), p = parents[v];
				in[v] = order[p];
				order[p] += out[v];
				order[v] = in[v] + 1;
			}
			for (int v = 0; v < n; ++v) out[v] += in[v], order[in[v]] = v;
		};
		if (ordered) { finish([](int v) { return v; }); return; }
		std::vector<int> start(std::size_t(n) + 1), adjacent(std::size_t(n) - 1);
		for (int p : parents) if (p >= 0) ++start[std::size_t(p) + 1];
		for (int v = 0; v < n; ++v) start[v + 1] += start[v];
		auto cursor = start;
		for (int v = 0; v < n; ++v) if (parents[v] >= 0) adjacent[cursor[parents[v]]++] = v;
		std::vector<int> traversal;
		traversal.reserve(n);
		traversal.push_back(root_);
		for (std::size_t i = 0; i < traversal.size(); ++i) {
			int v = traversal[i];
			for (int j = start[v]; j < start[v + 1]; ++j) traversal.push_back(adjacent[j]);
		}
		assert(traversal.size() == std::size_t(n));
		finish([&](int i) { return traversal[i]; });
	}
	std::pair<int, int> subtree(int v, bool edge = false) const { check(v); return {in[v] + int(edge), out[v]}; }
	bool is_ancestor(int u, int v) const {
		check(u); check(v);
		return in[u] <= in[v] && in[v] < out[u];
	}
};

#endif
