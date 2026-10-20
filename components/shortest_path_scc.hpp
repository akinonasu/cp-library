#ifndef SHORTEST_PATH_SCC_HPP
#define SHORTEST_PATH_SCC_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace negative_path_detail {
struct Components {
	std::vector<int> id, vertices;
	std::vector<std::size_t> offset{0};
};
template<class Graph>
Components reachable_components(const Graph &g, int source) {
	assert(std::size_t(g.size()) <= std::size_t(std::numeric_limits<int>::max()));
	int n = static_cast<int>(g.size());
	assert(0 <= source && source < n);
	Components result;
	result.id.assign(n, -1);
	std::vector<int> index(n, -1), low(n), pending;
	struct Frame { int vertex; std::size_t next; };
	std::vector<Frame> stack;
	int clock = 0, count = 0;
	auto enter = [&](int v) {
		index[v] = low[v] = clock++;
		pending.push_back(v);
		stack.push_back({v, 0});
	};
	enter(source);
	while (!stack.empty()) {
		int v = stack.back().vertex;
		if (stack.back().next < g[v].size()) {
			int to = g[v][stack.back().next++].first;
			assert(0 <= to && to < n);
			if (index[to] == -1) enter(to);
			else if (result.id[to] == -1) low[v] = std::min(low[v], index[to]);
		} else {
			stack.pop_back();
			if (low[v] == index[v]) {
				int u;
				do {
					u = pending.back();
					pending.pop_back();
					result.id[u] = count;
					result.vertices.push_back(u);
				} while (u != v);
				++count;
				result.offset.push_back(result.vertices.size());
			}
			if (!stack.empty()) {
				int parent = stack.back().vertex;
				low[parent] = std::min(low[parent], low[v]);
			}
		}
	}
	return result;
}
}

#endif
