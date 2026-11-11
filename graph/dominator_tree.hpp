#ifndef DOMINATOR_TREE_HPP
#define DOMINATOR_TREE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

class DominatorTree {
	int n_;
public:
	struct Edge { int from, to; };
	struct Result {
		int root = -1;
		std::vector<int> idom;
		int size() const { return int(idom.size()); }
		bool empty() const { return idom.empty(); }
		bool reachable(int v) const { assert(0 <= v && v < size()); return idom[v] != -1; }
		bool dominates(int u, int v) const {
			assert(0 <= u && u < size() && 0 <= v && v < size());
			return in_[u] != -1 && in_[v] != -1 && in_[u] <= in_[v] && in_[v] < out_[u];
		}
		std::vector<std::vector<int>> tree() const {
			std::vector<std::vector<int>> result(idom.size());
			for (int v = 0; v < size(); ++v) if (idom[v] != -1 && v != root) result[idom[v]].push_back(v);
			return result;
		}
	private:
		friend class DominatorTree;
		std::vector<int> in_, out_;
	};
private:
	std::vector<Edge> edges_;
public:
	explicit DominatorTree(int n = 0) : n_(n) { assert(n >= 0); }
	int size() const { return n_; }
	int edge_count() const { return int(edges_.size()); }
	void reserve(std::size_t count) { assert(count <= std::size_t(std::numeric_limits<int>::max())); edges_.reserve(count); }
	int add_edge(int from, int to) {
		assert(0 <= from && from < n_ && 0 <= to && to < n_);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max()));
		int id = edge_count();
		edges_.push_back({from, to});
		return id;
	}
	Edge get_edge(int id) const { assert(0 <= id && id < edge_count()); return edges_[id]; }
	const std::vector<Edge> &edges() const { return edges_; }
	void clear() { edges_.clear(); }
private:
	template<bool Queries>
	Result solve_impl(int root) const {
		assert(n_ ? 0 <= root && root < n_ : root == 0 || root == -1);
		Result result;
		if (!n_) return result;
		result.root = root;
		result.idom.assign(n_, -1);
		if constexpr (Queries) {
			result.in_.assign(n_, -1);
			result.out_.assign(n_, -1);
		}
		if (n_ == 1) {
			result.idom[0] = 0;
			if constexpr (Queries) result.in_[0] = 0, result.out_[0] = 1;
			return result;
		}
		std::vector<int> offset(std::size_t(n_) + 1);
		for (auto edge : edges_) ++offset[std::size_t(edge.from) + 1];
		for (int v = 0; v < n_; ++v) offset[v + 1] += offset[v];
		auto cursor = offset;
		std::vector<int> adjacent(edges_.size());
		for (auto edge : edges_) adjacent[cursor[edge.from]++] = edge.to;
		std::vector<int> number(n_, -1), vertex, parent, stack;
		vertex.reserve(n_); parent.reserve(n_); stack.reserve(n_);
		number[root] = 0;
		vertex.push_back(root); parent.push_back(0); stack.push_back(root);
		cursor[root] = offset[root];
		while (!stack.empty()) {
			int v = stack.back();
			if (cursor[v] == offset[v + 1]) { stack.pop_back(); continue; }
			int to = adjacent[cursor[v]++];
			if (number[to] != -1) continue;
			number[to] = int(vertex.size());
			vertex.push_back(to); parent.push_back(number[v]); stack.push_back(to);
			cursor[to] = offset[to];
		}
		int count = int(vertex.size());
		if (count == 1) {
			result.idom[root] = root;
			if constexpr (Queries) result.in_[root] = 0, result.out_[root] = 1;
			return result;
		}
		std::vector<int> predecessor_offset = std::move(offset);
		predecessor_offset.resize(std::size_t(count) + 1);
		std::fill(predecessor_offset.begin(), predecessor_offset.end(), 0);
		for (auto edge : edges_) {
			int from = number[edge.from], to = number[edge.to];
			if (from != -1 && to > 0 && from != to) ++predecessor_offset[to + 1];
		}
		for (int v = 0; v < count; ++v) predecessor_offset[v + 1] += predecessor_offset[v];
		cursor = predecessor_offset;
		std::vector<int> predecessor = std::move(adjacent);
		predecessor.resize(predecessor_offset.back());
		for (auto edge : edges_) {
			int from = number[edge.from], to = number[edge.to];
			if (from != -1 && to > 0 && from != to) predecessor[cursor[to]++] = from;
		}
		std::vector<int> semi(count), label(count), ancestor(count), immediate(count, -1), head(count, -1), next(count, -1);
		std::iota(semi.begin(), semi.end(), 0);
		label = semi; ancestor = semi;
		auto eval = [&](int v) {
			stack.clear();
			for (int x = v; ancestor[ancestor[x]] != ancestor[x]; x = ancestor[x]) stack.push_back(x);
			while (!stack.empty()) {
				int x = stack.back(), p = ancestor[x]; stack.pop_back();
				if (semi[label[p]] < semi[label[x]]) label[x] = label[p];
				ancestor[x] = ancestor[p];
			}
			return label[v];
		};
		for (int v = count - 1; v > 0; --v) {
			for (int i = predecessor_offset[v]; i < predecessor_offset[v + 1]; ++i)
				semi[v] = std::min(semi[v], semi[eval(predecessor[i])]);
			next[v] = head[semi[v]]; head[semi[v]] = v;
			ancestor[v] = parent[v];
			for (int u = head[parent[v]]; u != -1; u = next[u]) {
				int best = eval(u);
				immediate[u] = semi[best] < semi[u] ? best : parent[v];
			}
			head[parent[v]] = -1;
		}
		immediate[0] = 0;
		for (int v = 1; v < count; ++v)
			if (immediate[v] != semi[v]) immediate[v] = immediate[immediate[v]];
		result.idom[root] = root;
		if constexpr (Queries) {
			std::fill(parent.begin(), parent.end(), 1);
			for (int v = count - 1; v > 0; --v)
				parent[immediate[v]] += parent[v];
			result.in_[root] = 0;
			result.out_[root] = count;
			head[0] = 1;
		}
		for (int v = 1; v < count; ++v) {
			int p = immediate[v], original = vertex[v];
			result.idom[original] = vertex[p];
			if constexpr (Queries) {
				int position = head[p];
				head[p] += parent[v];
				head[v] = position + 1;
				result.in_[original] = position;
				result.out_[original] = position + parent[v];
			}
		}
		return result;
	}
public:
	Result solve(int root = 0) const { return solve_impl<true>(root); }
	std::vector<int> idom(int root = 0) const { return solve_impl<false>(root).idom; }
};

#endif
