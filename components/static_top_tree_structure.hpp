#ifndef STATIC_TOP_TREE_STRUCTURE_HPP
#define STATIC_TOP_TREE_STRUCTURE_HPP

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <utility>
#include <vector>
#include "../tree/hld.hpp"

namespace static_top_tree_detail {
struct Structure {
	enum Kind : unsigned char { Vertex, Compress, Rake };
	struct Node {
		int parent = -1, left = -1, right = -1, index = -1;
		Kind kind = Vertex;
	};
	std::vector<Node> nodes;
	std::vector<int> order, parent, edge_child;
	int root = -1, root_vertex = -1, paths = 0, points = 0, height = 0;
	Structure() = default;
	explicit Structure(const HLD& tree) { build(tree); }
	void build(const HLD& tree) {
		int n = tree.n;
		assert(n <= std::numeric_limits<int>::max() / 2);
		assert(int(tree.order.size()) == n);
		nodes.assign(n, {}), order.clear();
		nodes.reserve(std::max(0, 2 * n - 1)), order.reserve(std::max(0, 2 * n - 1));
		parent = tree.parent;
		edge_child.clear(), edge_child.reserve(tree.edges.size());
		for (auto [u, v] : tree.edges) edge_child.push_back(parent[u] == v ? u : v);
		paths = n, points = 0, root = -1, height = 0;
		root_vertex = n ? tree.order[0] : -1;
		for (int v = 0; v < n; ++v) nodes[v].index = v;
		std::vector<std::pair<int, int>> heads(n), stack, heap;
		auto join = [&](std::pair<int, int> a, std::pair<int, int> b, Kind kind) {
			int id = int(nodes.size());
			nodes.push_back({-1, a.second, b.second, kind == Compress ? paths++ : points++, kind});
			nodes[a.second].parent = nodes[b.second].parent = id;
			order.push_back(id);
			return std::pair<int, int>{std::max(a.first, b.first) + 1, id};
		};
		auto merge = [&] {
			auto b = stack.back(); stack.pop_back();
			auto a = stack.back(); stack.back() = join(a, b, Compress);
		};
		for (int i = n - 1; i >= 0; --i) {
			int head = tree.order[i];
			if (tree.head[head] != head) continue;
			stack.clear();
			for (int v = head; v != -1; v = tree.heavy[v]) {
				heap.clear();
				for (int j = tree.in[v] + 1; j < tree.out[v];) {
					int u = tree.order[j];
					if (u != tree.heavy[v]) heap.push_back(heads[u]);
					j = tree.out[u];
				}
				using Greater = std::greater<std::pair<int, int>>;
				std::make_heap(heap.begin(), heap.end(), Greater{});
				while (heap.size() > 1) {
					std::pop_heap(heap.begin(), heap.end(), Greater{});
					auto a = heap.back(); heap.pop_back();
					std::pop_heap(heap.begin(), heap.end(), Greater{});
					auto b = heap.back(); heap.back() = join(a, b, Rake);
					std::push_heap(heap.begin(), heap.end(), Greater{});
				}
				int h = 0;
				if (!heap.empty()) {
					nodes[v].left = heap[0].second;
					nodes[heap[0].second].parent = v;
					h = heap[0].first + 1;
				}
				order.push_back(v);
				stack.emplace_back(h, v);
				while (true) {
					int s = int(stack.size());
					if (s >= 3 && (stack[s - 3].first == stack[s - 2].first || stack[s - 3].first <= stack[s - 1].first)) {
						auto last = stack.back(); stack.pop_back();
						merge(), stack.push_back(last);
					} else if (s >= 2 && stack[s - 2].first <= stack[s - 1].first) merge();
					else break;
				}
			}
			while (stack.size() > 1) merge();
			heads[head] = stack[0];
			root = stack[0].second, height = stack[0].first;
		}
	}
};
}

#endif
