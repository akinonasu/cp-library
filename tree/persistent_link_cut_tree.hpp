#ifndef PERSISTENT_LINK_CUT_TREE_HPP
#define PERSISTENT_LINK_CUT_TREE_HPP

#include "../data_structure/persistent_array.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

template<class S, auto op, auto e, bool commutative = false>
class PersistentLinkCutTree {
	struct Empty {};
	using Back = std::conditional_t<commutative, Empty, S>;
	struct Node {
		int child[2] = {}, parent = 0, count = 1;
		bool reversed = false;
		S value, aggregate;
		[[no_unique_address]] Back backward;
		static Back make_back(const S &value) {
			if constexpr (commutative) return {};
			else return value;
		}
		explicit Node(const S &value) : value(value), aggregate(value), backward(make_back(value)) {}
	};
	using Storage = PersistentArray<Node>;
	struct Slot {
		std::optional<Node> node;
		bool dirty = false;
	};
	Storage storage;
	std::vector<Slot> cache;
	std::vector<int> touched, ancestors;

	static std::vector<S> initial_values(int n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
		return std::vector<S>(n, e());
	}
	static std::vector<int> forest_parents(int n, std::span<const std::pair<int, int>> edges) {
		assert(edges.size() < std::size_t(n) || (n == 0 && edges.empty()));
		std::vector<int> degree(n), neighbors(n), parent(n, -1), order;
		order.reserve(n);
		for (auto [u, v] : edges) {
			assert(0 <= u && u < n && 0 <= v && v < n && u != v);
			++degree[u], ++degree[v];
			neighbors[u] ^= v, neighbors[v] ^= u;
		}
		for (int v = 0; v < n; ++v) if (degree[v] <= 1) order.push_back(v);
		for (std::size_t i = 0; i < order.size(); ++i) {
			int v = order[i];
			if (!degree[v]) continue;
			int p = neighbors[v];
			parent[v] = p;
			degree[v] = 0;
			neighbors[p] ^= v;
			if (--degree[p] == 1) order.push_back(p);
		}
		assert(order.size() == std::size_t(n));
		return parent;
	}
	static std::vector<Node> initial_nodes(const std::vector<S> &values, std::span<const std::pair<int, int>> edges) {
		assert(values.size() < std::size_t(std::numeric_limits<int>::max()));
		auto parent = forest_parents(static_cast<int>(values.size()), edges);
		std::vector<Node> nodes;
		nodes.reserve(values.size() + 1);
		nodes.emplace_back(e());
		nodes[0].count = 0;
		for (std::size_t v = 0; v < values.size(); ++v) {
			nodes.emplace_back(values[v]);
			nodes.back().parent = parent[v] + 1;
		}
		return nodes;
	}
	int node(int v) const {
		assert(0 <= v && v < size());
		return v + 1;
	}

  public:
	using root_type = typename Storage::root_type;
	template<class T> struct result_type {
		root_type root;
		T value;
	};

  private:
	class Editor {
		PersistentLinkCutTree &owner;
		root_type version;
		Node &load(int x) {
			auto &slot = owner.cache[x];
			if (!slot.node) {
				owner.touched.push_back(x);
				slot.node.emplace(owner.storage.get(version, x));
			}
			return *slot.node;
		}
		bool is_root(int x) {
			int p = read(x).parent;
			return !p || (read(p).child[0] != x && read(p).child[1] != x);
		}
		void flip(int x) {
			if (!x) return;
			auto &v = write(x);
			std::swap(v.child[0], v.child[1]);
			if constexpr (!commutative) std::swap(v.aggregate, v.backward);
			v.reversed = !v.reversed;
		}
		void push(int x) {
			if (!read(x).reversed) return;
			flip(read(x).child[0]);
			flip(read(x).child[1]);
			write(x).reversed = false;
		}
		void rotate(int x) {
			int p = read(x).parent, g = read(p).parent;
			int d = read(p).child[1] == x, c = read(x).child[d ^ 1];
			if (!is_root(p)) write(g).child[read(g).child[1] == p] = x;
			write(x).parent = g;
			write(p).child[d] = c;
			if (c) write(c).parent = p;
			write(x).child[d ^ 1] = p;
			write(p).parent = x;
			pull(p);
			pull(x);
		}

	  public:
		Editor(PersistentLinkCutTree &owner, root_type version) : owner(owner), version(version) {
			assert(version <= owner.storage.node_count() && owner.touched.empty());
		}
		~Editor() {
			for (int x : owner.touched) {
				owner.cache[x].node.reset();
				owner.cache[x].dirty = false;
			}
			owner.touched.clear();
			owner.ancestors.clear();
		}
		const Node &read(int x) { return load(x); }
		Node &write(int x) {
			Node &value = load(x);
			owner.cache[x].dirty = true;
			return value;
		}
		root_type commit() {
			for (int x : owner.touched) if (owner.cache[x].dirty)
				version = owner.storage.set(version, x, std::move(*owner.cache[x].node));
			return version;
		}
		template<class T> result_type<T> finish(T value) { return {commit(), std::move(value)}; }
		void pull(int x) {
			auto &v = write(x);
			const auto &l = read(v.child[0]), &r = read(v.child[1]);
			v.count = l.count + r.count + 1;
			v.aggregate = op(op(l.aggregate, v.value), r.aggregate);
			if constexpr (!commutative) v.backward = op(op(r.backward, v.value), l.backward);
		}
		void splay(int x) {
			auto &path = owner.ancestors;
			path.clear();
			for (int y = x;; y = read(y).parent) {
				path.push_back(y);
				if (is_root(y)) break;
			}
			for (auto it = path.rbegin(); it != path.rend(); ++it) push(*it);
			while (!is_root(x)) {
				int p = read(x).parent, g = read(p).parent;
				if (!is_root(p)) rotate((read(p).child[0] == x) == (read(g).child[0] == p) ? p : x);
				rotate(x);
			}
		}
		int access(int x) {
			int last = 0;
			for (int y = x; y; y = read(y).parent) {
				splay(y);
				write(y).child[1] = last;
				if (last) write(last).parent = y;
				pull(y);
				last = y;
			}
			splay(x);
			return last;
		}
		void make_root(int x) { access(x); flip(x); }
		int root(int x) {
			access(x);
			push(x);
			while (read(x).child[0]) push(x = read(x).child[0]);
			splay(x);
			return x;
		}
		int expose(int u, int v) {
			make_root(u);
			access(v);
			assert(u == v || read(u).parent != 0);
			return v;
		}
		int select(int x, int k) {
			if (k < 0 || k >= read(x).count) return -1;
			for (;;) {
				push(x);
				int l = read(x).child[0], count = read(l).count;
				if (k == count) break;
				if (k < count) x = l;
				else k -= count + 1, x = read(x).child[1];
			}
			splay(x);
			return x - 1;
		}
	};

  public:
	explicit PersistentLinkCutTree(int n = 0, std::span<const std::pair<int, int>> edges = {})
		: PersistentLinkCutTree(initial_values(n), edges) {}
	explicit PersistentLinkCutTree(const std::vector<S> &values, std::span<const std::pair<int, int>> edges = {})
		: storage(initial_nodes(values, edges), Node(e())), cache(values.size() + 1) {}
	PersistentLinkCutTree(const PersistentLinkCutTree &) = delete;
	PersistentLinkCutTree &operator=(const PersistentLinkCutTree &) = delete;
	PersistentLinkCutTree(PersistentLinkCutTree &&) = delete;
	PersistentLinkCutTree &operator=(PersistentLinkCutTree &&) = delete;
	int size() const { return storage.size() - 1; }
	bool empty() const { return size() == 0; }
	root_type initial_root() const { return storage.initial_root(); }
	S get(root_type version, int v) const { return storage.get(version, node(v)).value; }
	root_type set(root_type version, int v, S value) {
		Editor edit(*this, version);
		int x = node(v);
		edit.access(x);
		edit.write(x).value = std::move(value);
		edit.pull(x);
		return edit.commit();
	}
	root_type make_root(root_type version, int v) {
		Editor edit(*this, version);
		edit.make_root(node(v));
		return edit.commit();
	}
	root_type build_forest(root_type version, std::span<const std::pair<int, int>> edges) {
		auto parent = forest_parents(size(), edges);
		Editor edit(*this, version);
		for (int v = 0; v < size(); ++v) {
			int x = v + 1;
			Node replacement(edit.read(x).value);
			replacement.parent = parent[v] + 1;
			edit.write(x) = std::move(replacement);
		}
		return edit.commit();
	}
	result_type<int> root(root_type version, int v) {
		Editor edit(*this, version);
		return edit.finish(edit.root(node(v)) - 1);
	}
	result_type<bool> same(root_type version, int u, int v) {
		Editor edit(*this, version);
		u = node(u), v = node(v);
		if (u == v) return edit.finish(true);
		edit.access(u);
		edit.access(v);
		return edit.finish(edit.read(u).parent != 0);
	}
	result_type<bool> link(root_type version, int u, int v) {
		Editor edit(*this, version);
		u = node(u), v = node(v);
		if (u == v) return edit.finish(false);
		edit.make_root(u);
		edit.access(v);
		if (edit.read(u).parent) return edit.finish(false);
		edit.write(u).parent = v;
		return edit.finish(true);
	}
	result_type<bool> cut(root_type version, int u, int v) {
		Editor edit(*this, version);
		u = node(u), v = node(v);
		if (u == v) return edit.finish(false);
		edit.make_root(u);
		edit.access(v);
		if (edit.read(v).child[0] != u || edit.read(u).child[1]) return edit.finish(false);
		edit.write(v).child[0] = 0;
		edit.write(u).parent = 0;
		edit.pull(v);
		return edit.finish(true);
	}
	result_type<S> prod(root_type version, int u, int v) {
		Editor edit(*this, version);
		int x = edit.expose(node(u), node(v));
		return edit.finish(edit.read(x).aggregate);
	}
	result_type<int> path_size(root_type version, int u, int v) {
		Editor edit(*this, version);
		int x = edit.expose(node(u), node(v));
		return edit.finish(edit.read(x).count);
	}
	result_type<int> distance(root_type version, int u, int v) {
		auto result = path_size(version, u, v);
		--result.value;
		return result;
	}
	result_type<int> jump(root_type version, int u, int v, int k) {
		Editor edit(*this, version);
		int x = edit.expose(node(u), node(v));
		return edit.finish(edit.select(x, k));
	}
	result_type<int> lca(root_type version, int r, int u, int v) {
		Editor edit(*this, version);
		r = node(r), u = node(u), v = node(v);
		edit.make_root(r);
		edit.access(u);
		assert(r == u || edit.read(r).parent != 0);
		int w = edit.access(v);
		assert(u == v || edit.read(u).parent != 0);
		return edit.finish(w - 1);
	}
	void reserve(std::size_t count) { storage.reserve(count); }
	std::size_t node_count() const { return storage.node_count(); }
	std::size_t memory_usage() const {
		return storage.memory_usage() + cache.capacity() * sizeof(Slot)
			+ (touched.capacity() + ancestors.capacity()) * sizeof(int);
	}
};

#endif
