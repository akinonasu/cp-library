#ifndef FLOW_GRAPH_HPP
#define FLOW_GRAPH_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace flow_detail {
template<class Cap> struct Graph {
	struct Arc { int to; Cap cap; };
	int n;
	std::vector<Arc> arcs;
	std::vector<int> start, order;
	bool built = false;
	explicit Graph(int n = 0) : n(n) { assert(n >= 0); }
	void reserve(std::size_t m) { assert(m <= std::size_t(std::numeric_limits<int>::max() / 2)); arcs.reserve(2 * m); }
	int add_edge(int from, int to, Cap cap) {
		assert(0 <= from && from < n && 0 <= to && to < n && cap >= 0);
		assert(arcs.size() <= std::size_t(std::numeric_limits<int>::max() - 2));
		int id = int(arcs.size());
		arcs.push_back({to, cap});
		arcs.push_back({from, Cap(0)});
		built = false;
		return id / 2;
	}
	void build() {
		if (built) return;
		start.assign(std::size_t(n) + 1, 0);
		for (std::size_t i = 0; i < arcs.size(); ++i) ++start[arcs[i ^ 1].to + 1];
		for (int v = 0; v < n; ++v) start[v + 1] += start[v];
		order.resize(arcs.size());
		auto next = start;
		for (int i = 0; i < int(arcs.size()); ++i) order[next[arcs[i ^ 1].to]++] = i;
		built = true;
	}
	void reset_flow() {
		for (std::size_t i = 0; i < arcs.size(); i += 2) arcs[i].cap += arcs[i + 1].cap, arcs[i + 1].cap = 0;
	}
};
}

#endif
