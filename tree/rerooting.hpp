#ifndef REROOTING_HPP
#define REROOTING_HPP

#include "../components/undirected_csr.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template<class T, class E = T>
class Rerooting {
	int n_;
	bool built_ = false;
	std::vector<std::pair<int, int>> edges_;
	std::vector<T> values_;
	std::vector<E> messages_;

  public:
	using Vertex = T;
	using Edge = E;
	explicit Rerooting(int n = 0) : n_(n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
		edges_.reserve(std::max(0, n - 1));
	}
	int size() const { return n_; }
	bool empty() const { return n_ == 0; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	int add_edge(int u, int v) {
		assert(0 <= u && u < n_ && 0 <= v && v < n_ && u != v);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		built_ = false;
		edges_.emplace_back(u, v);
		return edge_count() - 1;
	}
	const std::pair<int, int>& edge(int id) const {
		assert(0 <= id && id < edge_count());
		return edges_[id];
	}
	const T& operator[](int v) const {
		assert(built_ && 0 <= v && v < n_);
		return values_[v];
	}
	const std::vector<T>& values() const {
		assert(built_);
		return values_;
	}
	const E& edge_dp(int id, bool reverse = false) const {
		assert(built_ && 0 <= id && id < edge_count());
		return messages_[2 * id + reverse];
	}
	template<class Merge, class AddVertex, class AddEdge>
	const std::vector<T>& build(E unit, Merge&& merge, AddVertex&& add_vertex, AddEdge&& add_edge, int root = 0) {
		assert(n_ == 0 || (0 <= root && root < n_));
		built_ = false;
		if constexpr (std::is_default_constructible_v<E>) messages_.resize(2 * edges_.size());
		else messages_.assign(2 * edges_.size(), unit);
		if (n_ == 0) {
			values_.clear();
			built_ = true;
			return values_;
		}
		UndirectedCSR graph(n_, edges_);
		for (int v = 0; v < n_; ++v)
			for (int i = graph.start[v]; i < graph.start[v + 1]; ++i) {
				int id = graph.adj[i].id;
				graph.adj[i].id = 2 * id + (edges_[id].second == v);
			}
		std::vector<int> parent(n_, -2), parent_edge(n_, -1), order;
		order.reserve(n_);
		auto visit = [&](int s) {
			if (parent[s] != -2) return;
			parent[s] = -1;
			order.push_back(s);
			for (std::size_t i = order.size() - 1; i < order.size(); ++i) {
				int v = order[i];
				for (auto [to, id] : graph[v]) {
					if (id == parent_edge[v]) continue;
					assert(parent[to] == -2);
					if (parent[to] != -2) continue;
					parent[to] = v, parent_edge[to] = id ^ 1;
					order.push_back(to);
				}
			}
		};
		visit(root);
		for (int v = 0; v < n_; ++v) visit(v);
		for (auto it = order.rbegin(); it != order.rend(); ++it) {
			int v = *it, id = parent_edge[v];
			if (id == -1) continue;
			E value = unit;
			for (auto arc : graph[v])
				if (arc.id != id) value = merge(value, messages_[arc.id ^ 1]);
			messages_[id] = add_edge(add_vertex(value, v), v, parent[v], id >> 1);
		}
		if constexpr (std::is_default_constructible_v<T>) values_.resize(n_);
		else values_.assign(n_, unit);
		int maximum = 0;
		for (int v = 0; v < n_; ++v) maximum = std::max(maximum, graph.start[v + 1] - graph.start[v]);
		std::vector<E> prefix(maximum, unit);
		for (int v : order) {
			auto arcs = graph[v];
			int degree = static_cast<int>(arcs.size());
			if (degree == 0) {
				values_[v] = add_vertex(unit, v);
				continue;
			}
			auto incoming = [&](int i) -> const E& { return messages_[arcs[i].id ^ 1]; };
			if (degree > 1) prefix[1] = incoming(0);
			for (int i = 1; i + 1 < degree; ++i) prefix[i + 1] = merge(prefix[i], incoming(i));
			values_[v] = add_vertex(degree == 1 ? incoming(0) : merge(prefix[degree - 1], incoming(degree - 1)), v);
			E suffix = unit;
			for (int i = degree - 1; i >= 0; --i) {
				auto [to, id] = arcs[i];
				if (id != parent_edge[v]) {
					E value = i == degree - 1 ? prefix[i] : i == 0 ? suffix : merge(prefix[i], suffix);
					messages_[id] = add_edge(add_vertex(value, v), v, to, id >> 1);
				}
				if (i > 0) suffix = i == degree - 1 ? incoming(i) : merge(incoming(i), suffix);
			}
		}
		built_ = true;
		return values_;
	}
};

template<class E, class V, auto merge, auto e, auto put_edge, auto put_vertex>
class RerootingDP {
	Rerooting<V, E> tree_;
	std::vector<std::pair<int, int>> indices_;
	std::vector<V> branches_;
	std::vector<int> parent_arc_, component_, in_, out_, child_start_, children_;
	int root_ = 0;
	bool built_ = false, rerooted_ = false;

  public:
	using Edge = E;
	using Vertex = V;

	explicit RerootingDP(int n = 0) : tree_(n) {}
	int size() const { return tree_.size(); }
	bool empty() const { return tree_.empty(); }
	int edge_count() const { return tree_.edge_count(); }
	int add_edge(int u, int v, int idx, int xdi) {
		int id = tree_.add_edge(u, v);
		indices_.emplace_back(idx, xdi);
		built_ = false;
		rerooted_ = false;
		return id;
	}
	std::vector<V> build(int root = 0) {
		assert(empty() || (0 <= root && root < size()));
		built_ = false;
		rerooted_ = false;
		root_ = root;
		if (empty()) {
			built_ = true;
			return {};
		}
		std::vector<std::pair<int, int>> edges;
		edges.reserve(edge_count());
		for (int id = 0; id < edge_count(); ++id) edges.push_back(tree_.edge(id));
		UndirectedCSR graph(size(), edges);
		std::vector<int> parent(size(), -2), order, stack;
		parent_arc_.assign(size(), -1);
		component_.resize(size());
		in_.resize(size()), out_.assign(size(), 1);
		order.reserve(size());
		stack.reserve(size());
		auto visit = [&](int s) {
			if (parent[s] != -2) return;
			parent[s] = -1;
			stack.push_back(s);
			while (!stack.empty()) {
				int v = stack.back();
				stack.pop_back();
				in_[v] = static_cast<int>(order.size());
				component_[v] = s;
				order.push_back(v);
				auto arcs = graph[v];
				for (auto it = arcs.rbegin(); it != arcs.rend(); ++it) {
					auto arc = *it;
					if (arc.to == parent[v]) continue;
					assert(parent[arc.to] == -2);
					if (parent[arc.to] != -2) continue;
					parent[arc.to] = v;
					parent_arc_[arc.to] = 2 * arc.id + (arc.to == edges[arc.id].second);
					stack.push_back(arc.to);
				}
			}
		};
		visit(root);
		for (int v = 0; v < size(); ++v) visit(v);
		for (auto it = order.rbegin(); it != order.rend(); ++it)
			if (parent[*it] != -1) out_[parent[*it]] += out_[*it];
		child_start_.assign(size() + 1, 0);
		children_.clear();
		children_.reserve(edge_count());
		for (int v = 0; v < size(); ++v) {
			out_[v] += in_[v];
			for (auto arc : graph[v]) if (parent[arc.to] == v) children_.push_back(arc.to);
			child_start_[v + 1] = static_cast<int>(children_.size());
		}
		std::vector<V> result(size(), put_vertex(e(), root));
		for (auto it = order.rbegin(); it != order.rend(); ++it) {
			int v = *it;
			E value = e();
			for (auto arc : graph[v]) if (parent[arc.to] == v) {
				int idx = v == edges[arc.id].first ? indices_[arc.id].first : indices_[arc.id].second;
				value = merge(value, put_edge(result[arc.to], idx));
			}
			result[v] = put_vertex(value, v);
		}
		built_ = true;
		return result;
	}
	std::vector<V> reroot() {
		assert(built_);
		rerooted_ = false;
		branches_.clear();
		if (edge_count()) branches_.assign(2 * indices_.size(), put_vertex(e(), root_));
		auto answer = tree_.build(e(), merge, put_vertex, [&](const V& value, int from, int, int id) {
			branches_[2 * id + (from == tree_.edge(id).second)] = value;
			int idx = from == tree_.edge(id).first ? indices_[id].second : indices_[id].first;
			return put_edge(value, idx);
		}, root_);
		rerooted_ = true;
		return answer;
	}
	const V& subtree(int u, int v) const {
		assert(rerooted_ && 0 <= u && u < size() && 0 <= v && v < size());
		assert(component_[u] == component_[v]);
		if (u == v) return tree_[v];
		if (!(in_[v] <= in_[u] && in_[u] < out_[v])) return branches_[parent_arc_[v]];
		int l = child_start_[v], r = child_start_[v + 1];
		while (l < r) {
			int m = l + (r - l) / 2;
			if (in_[children_[m]] <= in_[u]) l = m + 1;
			else r = m;
		}
		return branches_[parent_arc_[children_[l - 1]] ^ 1];
	}
};

#endif
