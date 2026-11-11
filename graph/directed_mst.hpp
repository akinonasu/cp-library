#ifndef DIRECTED_MST_HPP
#define DIRECTED_MST_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

template<class Cost = std::int64_t, class Sum = Cost>
class DirectedMST {
	static_assert(std::is_integral_v<Cost> && std::is_signed_v<Cost> && sizeof(Cost) <= 8);
	static_assert((std::is_integral_v<Sum> || std::is_same_v<Sum, __int128_t>) && std::numeric_limits<Sum>::is_signed);
	static_assert(std::numeric_limits<Cost>::digits <= std::numeric_limits<Sum>::digits);
	using Wide = __int128_t;
	using HeapCost = std::conditional_t<(sizeof(Cost) <= 4), std::int64_t, Wide>;
	int n_;
public:
	struct Edge { int from, to; Cost cost; };
	struct Result {
		Sum cost = 0;
		std::vector<int> parent, parent_edge, edges;
	};
private:
	std::vector<Edge> edges_;
	struct HeapNode {
		HeapCost key, lazy = 0;
		int left = -1, right = -1, edge, rank = 1;
	};
	struct Heap {
		std::vector<HeapNode> nodes;
		void add(int v, HeapCost value) {
			if (v != -1) nodes[v].key += value, nodes[v].lazy += value;
		}
		void push(int v) {
			HeapCost value = nodes[v].lazy;
			if (value) add(nodes[v].left, value), add(nodes[v].right, value), nodes[v].lazy = 0;
		}
		int rank(int v) const { return v == -1 ? 0 : nodes[v].rank; }
		int merge(int a, int b) {
			std::array<int, 64> path;
			int depth = 0;
			while (a != -1 && b != -1) {
				if (nodes[b].key < nodes[a].key || (nodes[b].key == nodes[a].key && nodes[b].edge < nodes[a].edge)) std::swap(a, b);
				push(a);
				assert(depth < int(path.size()));
				path[depth++] = a;
				a = nodes[a].right;
			}
			int result = a == -1 ? b : a;
			while (depth) {
				int v = path[--depth];
				nodes[v].right = result;
				if (rank(nodes[v].left) < rank(nodes[v].right)) std::swap(nodes[v].left, nodes[v].right);
				nodes[v].rank = rank(nodes[v].right) + 1;
				result = v;
			}
			return result;
		}
		int pop(int v) { push(v); return merge(nodes[v].left, nodes[v].right); }
	};
public:
	explicit DirectedMST(int n = 0) : n_(n) { assert(0 <= n && n <= std::numeric_limits<int>::max() / 2); }
	int size() const { return n_; }
	int edge_count() const { return int(edges_.size()); }
	void reserve(std::size_t count) { assert(count <= std::size_t(std::numeric_limits<int>::max())); edges_.reserve(count); }
	int add_edge(int from, int to, Cost cost) {
		assert(0 <= from && from < n_ && 0 <= to && to < n_);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max()));
		int id = edge_count();
		edges_.push_back({from, to, cost});
		return id;
	}
	Edge get_edge(int id) const { assert(0 <= id && id < edge_count()); return edges_[id]; }
	const std::vector<Edge> &edges() const { return edges_; }
	void clear() { edges_.clear(); }
	std::optional<Result> solve(int root = 0) const {
		assert(n_ ? 0 <= root && root < n_ : root == 0 || root == -1);
		Result result;
		if (!n_) return result;
		if (edges_.size() < std::size_t(n_ - 1)) return std::nullopt;
		result.parent.assign(n_, root);
		result.parent_edge.assign(n_, -1);
		if (n_ == 1) return result;
		std::size_t capacity = std::size_t(n_) * 2;
		std::vector<int> representative(capacity), heap(capacity, -1), seen(capacity, -1), chosen(capacity, -1);
		std::vector<int> upper(capacity, -1), first(capacity, -1), sibling(capacity, -1);
		for (int v = 0; v < n_; ++v) representative[v] = v;
		auto find = [&](int v) {
			int r = v;
			while (representative[r] != r) r = representative[r];
			while (representative[v] != v) { int p = representative[v]; representative[v] = r; v = p; }
			return r;
		};
		Heap incoming;
		incoming.nodes.reserve(edges_.size());
		for (int id = 0; id < edge_count(); ++id) {
			const auto &e = edges_[id];
			if (e.from == e.to || e.to == root) continue;
			int node = int(incoming.nodes.size());
			incoming.nodes.push_back({HeapCost(e.cost), 0, -1, -1, id, 1});
			heap[e.to] = incoming.merge(heap[e.to], node);
		}
		seen[root] = n_;
		int count = n_;
		std::vector<int> path;
		path.reserve(n_);
		for (int start = 0; start < n_; ++start) {
			int v = find(start);
			path.clear();
			while (seen[v] == -1) {
				seen[v] = start;
				path.push_back(v);
				int h = heap[v];
				while (h != -1 && find(edges_[incoming.nodes[h].edge].from) == v) h = incoming.pop(h);
				if (h == -1) return std::nullopt;
				int id = chosen[v] = incoming.nodes[h].edge;
				incoming.add(h, -incoming.nodes[h].key);
				heap[v] = incoming.pop(h);
				int next = find(edges_[id].from);
				if (seen[next] == start) {
					int contracted = count++;
					representative[contracted] = contracted;
					int child;
					do {
						child = path.back(); path.pop_back();
						upper[child] = representative[child] = contracted;
						sibling[child] = first[contracted]; first[contracted] = child;
						heap[contracted] = incoming.merge(heap[contracted], heap[child]);
					} while (child != next);
					v = contracted;
				} else v = next;
			}
		}
		std::vector<int> left = std::move(seen), right = std::move(heap), stack = std::move(representative);
		left.resize(count);
		right.resize(count);
		stack.clear();
		int position = 0;
		for (int top = 0; top < count; ++top) if (upper[top] == -1) {
			stack.push_back(top);
			while (!stack.empty()) {
				int v = stack.back(); stack.pop_back();
				if (v < 0) { right[~v] = position; continue; }
				left[v] = position;
				if (v < n_) { ++position; right[v] = position; }
				else {
					stack.push_back(~v);
					for (int child = first[v]; child != -1; child = sibling[child]) stack.push_back(child);
				}
			}
		}
		for (int top = 0; top < count; ++top) if (upper[top] == -1 && top != root) stack.push_back(top);
		while (!stack.empty()) {
			int v = stack.back(), id = chosen[v]; stack.pop_back();
			if (v < n_) { result.parent_edge[v] = id; result.parent[v] = edges_[id].from; }
			else {
				int target = left[edges_[id].to];
				for (int child = first[v]; child != -1; child = sibling[child]) {
					if (left[child] <= target && target < right[child]) chosen[child] = id;
					stack.push_back(child);
				}
			}
		}
		Wide total = 0;
		result.edges.reserve(std::size_t(n_) - 1);
		for (int v = 0; v < n_; ++v) if (v != root) {
			int id = result.parent_edge[v];
			total += Wide(edges_[id].cost);
			result.edges.push_back(id);
		}
		if (total < Wide(std::numeric_limits<Sum>::lowest()) || total > Wide(std::numeric_limits<Sum>::max())) throw std::overflow_error("DirectedMST cost");
		result.cost = Sum(total);
		return result;
	}
};

#endif
