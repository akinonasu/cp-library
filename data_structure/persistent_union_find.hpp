#ifndef PERSISTENT_UNION_FIND_HPP
#define PERSISTENT_UNION_FIND_HPP

#include "persistent_array.hpp"
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

class PersistentUnionFind {
	PersistentArray<int> parent;
  public:
	struct root_type {
		PersistentArray<int>::root_type data = 0;
		int components = 0;
		bool operator==(const root_type &) const = default;
	};
	explicit PersistentUnionFind(int n = 0) : parent(n, -1) {}
	int vertex_count() const { return parent.size(); }
	root_type initial_root() const { return {parent.initial_root(), vertex_count()}; }
	int leader(root_type root, int x) const {
		int p;
		while ((p = parent.get(root.data, x)) >= 0) x = p;
		return x;
	}
	bool same(root_type root, int a, int b) const { return leader(root, a) == leader(root, b); }
	int size(root_type root, int x) const { return -parent.get(root.data, leader(root, x)); }
	int component_count(root_type root) const { return root.components; }
	root_type merge(root_type root, int a, int b) {
		a = leader(root, a); b = leader(root, b);
		if (a == b) return root;
		int x = parent.get(root.data, a), y = parent.get(root.data, b);
		if (x > y) { std::swap(a, b); std::swap(x, y); }
		root.data = parent.set(root.data, a, x + y);
		root.data = parent.set(root.data, b, a);
		--root.components;
		return root;
	}
	std::vector<std::vector<int>> groups(root_type root) const {
		std::vector<std::vector<int>> result(vertex_count());
		for (int i = 0; i < vertex_count(); ++i) result[leader(root, i)].push_back(i);
		std::vector<std::vector<int>> groups;
		for (auto &g : result) if (!g.empty()) groups.push_back(std::move(g));
		return groups;
	}
	void reserve(std::size_t count) { parent.reserve(count); }
	std::size_t node_count() const { return parent.node_count(); }
	std::size_t memory_usage() const { return parent.memory_usage(); }
};

#endif
