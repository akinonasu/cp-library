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
	int n_;
	std::vector<std::pair<int, int>> edges_, indices_;
	std::vector<UndirectedCSR::Arc> arcs_;
	std::vector<int> start_, order_, parent_arc_, component_, in_, out_, child_start_, children_;
	std::vector<E> messages_;
	std::vector<V> values_, branches_;
	bool built_ = false, rerooted_ = false;

	E lift(const V& value, int id) const {
		int idx = id & 1 ? indices_[id >> 1].first : indices_[id >> 1].second;
		return put_edge(value, idx);
	}

  public:
	using Edge = E;
	using Vertex = V;

	explicit RerootingDP(int n = 0) : n_(n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
		edges_.reserve(std::max(0, n - 1));
		indices_.reserve(std::max(0, n - 1));
	}
	int size() const { return n_; }
	bool empty() const { return n_ == 0; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	int add_edge(int u, int v, int idx, int xdi) {
		assert(0 <= u && u < n_ && 0 <= v && v < n_ && u != v);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		edges_.emplace_back(u, v);
		indices_.emplace_back(idx, xdi);
		built_ = rerooted_ = false;
		return edge_count() - 1;
	}
	std::vector<V> build(int root = 0) {
		assert(empty() || (0 <= root && root < n_));
		built_ = rerooted_ = false;
		if (empty()) {
			values_.clear();
			built_ = true;
			return {};
		}
		start_.assign(std::size_t(n_) + 1, 0);
		for (auto [u, v] : edges_) ++start_[u + 1], ++start_[v + 1];
		for (int v = 0; v < n_; ++v) start_[v + 1] += start_[v];
		arcs_.resize(2 * edges_.size());
		std::vector<int> cursor = start_;
		for (int id = 0; id < edge_count(); ++id) {
			auto [u, v] = edges_[id];
			arcs_[cursor[u]++] = {v, 2 * id};
			arcs_[cursor[v]++] = {u, 2 * id + 1};
		}
		parent_arc_.assign(n_, -2);
		component_.resize(n_);
		in_.resize(n_), out_.assign(n_, 1);
		order_.clear();
		order_.reserve(n_);
		std::vector<int> stack;
		stack.reserve(n_);
		auto visit = [&](int s) {
			if (parent_arc_[s] != -2) return;
			parent_arc_[s] = -1;
			stack.push_back(s);
			while (!stack.empty()) {
				int v = stack.back();
				stack.pop_back();
				in_[v] = static_cast<int>(order_.size());
				component_[v] = s;
				order_.push_back(v);
				for (int i = start_[v + 1]; i-- > start_[v];) {
					auto arc = arcs_[i];
					if (arc.id == parent_arc_[v]) continue;
					assert(parent_arc_[arc.to] == -2);
					if (parent_arc_[arc.to] != -2) continue;
					parent_arc_[arc.to] = arc.id ^ 1;
					stack.push_back(arc.to);
				}
			}
		};
		visit(root);
		for (int v = 0; v < n_; ++v) visit(v);
		for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
			int id = parent_arc_[*it];
			if (id != -1) {
				auto [u, v] = edges_[id >> 1];
				out_[id & 1 ? u : v] += out_[*it];
			}
		}
		child_start_.assign(std::size_t(n_) + 1, 0);
		children_.clear();
		children_.reserve(edge_count());
		for (int v = 0; v < n_; ++v) {
			out_[v] += in_[v];
			for (int i = start_[v]; i < start_[v + 1]; ++i)
				if (arcs_[i].id != parent_arc_[v]) children_.push_back(arcs_[i].to);
			child_start_[v + 1] = static_cast<int>(children_.size());
		}
		E unit = e();
		if constexpr (std::is_default_constructible_v<E>) messages_.resize(arcs_.size());
		else messages_.assign(arcs_.size(), unit);
		if constexpr (std::is_default_constructible_v<V>) {
			values_.resize(n_);
			branches_.resize(arcs_.size());
		} else {
			V initial = put_vertex(unit, root);
			values_.assign(n_, initial);
			branches_.assign(arcs_.size(), initial);
		}
		for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
			int v = *it, id = parent_arc_[v];
			int l = child_start_[v], r = child_start_[v + 1];
			E value = l == r ? unit : messages_[parent_arc_[children_[l++]]];
			for (; l < r; ++l) value = merge(value, messages_[parent_arc_[children_[l]]]);
			values_[v] = put_vertex(value, v);
			if (id != -1) {
				branches_[id] = values_[v];
				messages_[id] = lift(values_[v], id);
			}
		}
		built_ = true;
		return values_;
	}
	std::vector<V> reroot() {
		assert(built_);
		if (rerooted_) return values_;
		E unit = e();
		int maximum = 0;
		for (int v = 0; v < n_; ++v) maximum = std::max(maximum, start_[v + 1] - start_[v]);
		std::vector<E> prefix(maximum, unit);
		for (int v : order_) {
			int start = start_[v], degree = start_[v + 1] - start;
			if (degree == 0) continue;
			auto incoming = [&](int i) -> const E& { return messages_[arcs_[start + i].id ^ 1]; };
			if (degree > 1) prefix[1] = incoming(0);
			for (int i = 1; i + 1 < degree; ++i) prefix[i + 1] = merge(prefix[i], incoming(i));
			values_[v] = put_vertex(degree == 1 ? incoming(0) : merge(prefix[degree - 1], incoming(degree - 1)), v);
			E suffix = unit;
			for (int i = degree - 1; i >= 0; --i) {
				int id = arcs_[start + i].id;
				if (id != parent_arc_[v]) {
					E value = i == degree - 1 ? prefix[i] : i == 0 ? suffix : merge(prefix[i], suffix);
					branches_[id] = put_vertex(value, v);
					messages_[id] = lift(branches_[id], id);
				}
				if (i > 0) suffix = i == degree - 1 ? incoming(i) : merge(incoming(i), suffix);
			}
		}
		rerooted_ = true;
		return values_;
	}
	const V& subtree(int u, int v) const {
		assert(rerooted_ && 0 <= u && u < n_ && 0 <= v && v < n_);
		assert(component_[u] == component_[v]);
		if (u == v) return values_[v];
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
