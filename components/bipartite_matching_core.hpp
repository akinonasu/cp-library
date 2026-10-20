#ifndef BIPARTITE_MATCHING_CORE_HPP
#define BIPARTITE_MATCHING_CORE_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <utility>
#include <vector>

namespace bipartite_matching_detail {
struct CSR {
	std::vector<int> start, adj;
	CSR(int n, std::span<const std::pair<int, int>> edges, bool transpose) : start(n + 1), adj(edges.size()) {
		for (auto [u, v] : edges) ++start[(transpose ? v : u) + 1];
		std::partial_sum(start.begin(), start.end(), start.begin());
		auto next = start;
		for (auto [u, v] : edges) adj[next[transpose ? v : u]++] = transpose ? u : v;
	}
	CSR(int n, const CSR& graph) : start(n + 1), adj(graph.adj.size()) {
		for (int v : graph.adj) ++start[v + 1];
		std::partial_sum(start.begin(), start.end(), start.begin());
		auto next = start;
		for (int u = 0; u + 1 < int(graph.start.size()); ++u)
			for (int e = graph.start[u]; e < graph.start[u + 1]; ++e) adj[next[graph.adj[e]]++] = u;
	}
};

struct Result { int added; bool maximum; };

inline Result hopcroft_karp(const CSR& graph, std::vector<int>& left, std::vector<int>& right,
	int phases = std::numeric_limits<int>::max(), int max_distance = std::numeric_limits<int>::max()) {
	int n = int(left.size()), added = 0;
	std::vector<int> distance(n), queue(n), next(n), path(n);
	for (int phase = 0; phase < phases; ++phase) {
		int tail = 0, limit = n + 1;
		for (int u = 0; u < n; ++u) {
			distance[u] = left[u] < 0 ? 0 : -1;
			if (left[u] < 0) queue[tail++] = u;
		}
		for (int head = 0; head < tail; ++head) {
			int u = queue[head];
			if (distance[u] >= limit) break;
			for (int e = graph.start[u]; e < graph.start[u + 1]; ++e) {
				int w = right[graph.adj[e]];
				if (w < 0) limit = distance[u] + 1;
				else if (distance[w] < 0) distance[w] = distance[u] + 1, queue[tail++] = w;
			}
		}
		if (limit == n + 1) return {added, true};
		if (limit > max_distance) return {added, false};
		std::copy_n(graph.start.begin(), n, next.begin());
		for (int root = 0; root < n; ++root) if (left[root] < 0 && distance[root] == 0) {
			int depth = 1;
			path[0] = root;
			while (depth) {
				int u = path[depth - 1];
				bool descend = false, found = false;
				while (next[u] < graph.start[u + 1]) {
					int v = graph.adj[next[u]++], w = right[v];
					if (w < 0) {
						if (distance[u] + 1 != limit) continue;
						while (depth) {
							int x = path[--depth], previous = left[x];
							left[x] = v, right[v] = x;
							distance[x] = -1;
							v = previous;
						}
						++added;
						found = true;
						break;
					}
					if (distance[w] == distance[u] + 1 && distance[w] < limit) {
						path[depth++] = w;
						descend = true;
						break;
					}
				}
				if (found) break;
				if (!descend) distance[u] = -1, --depth;
			}
		}
	}
	return {added, false};
}

inline int warm_start(const CSR& graph, std::vector<int>& left, std::vector<int>& right, int multiplier) {
	assert(multiplier >= 0);
	int l = int(left.size()), r = int(right.size()), n = l + r, added = 0;
	int deficit = int(std::count(left.begin(), left.end(), -1));
	if (!deficit) return 0;
	CSR reverse(r, graph);
	std::vector<int> dl(l), dr(r), bfs(l), active(r);
	std::uint64_t work = 0, budget = std::uint64_t(graph.adj.size() + n) * multiplier;
	int head = 0, tail = 0, count = 0, steps = 0;
	bool first = true;
	for (;;) {
		if (!steps) {
			work += n;
			if (work > budget) return added;
			int end = 0;
			std::fill(dr.begin(), dr.end(), n);
			for (int u = 0; u < l; ++u) {
				dl[u] = left[u] < 0 ? 0 : n;
				if (left[u] < 0) bfs[end++] = u;
			}
			for (int begin = 0; begin < end; ++begin) {
				int u = bfs[begin];
				work += graph.start[u + 1] - graph.start[u];
				if (work > budget) return added;
				for (int e = graph.start[u]; e < graph.start[u + 1]; ++e) {
					int v = graph.adj[e];
					if (dr[v] <= dl[u] + 1) continue;
					dr[v] = dl[u] + 1;
					int w = right[v];
					if (w >= 0 && dl[w] == n) dl[w] = dr[v] + 1, bfs[end++] = w;
				}
			}
			if (first) {
				for (int v = 0; v < r; ++v) if (right[v] < 0 && dr[v] < n) active[count++] = v;
				tail = count == r ? 0 : count;
				first = false;
			}
		}
		if (!count) return added;
		int v = active[head++], best = -1, minimum = n;
		if (head == r) head = 0;
		--count;
		work += reverse.start[v + 1] - reverse.start[v] + 1;
		if (work > budget) return added;
		for (int e = reverse.start[v]; e < reverse.start[v + 1]; ++e) {
			int u = reverse.adj[e];
			if (dl[u] < minimum) minimum = dl[u], best = u;
		}
		if (best >= 0) {
			int previous = left[best];
			if (previous < 0) ++added;
			else {
				right[previous] = -1;
				active[tail++] = previous;
				if (tail == r) tail = 0;
				++count;
			}
			left[best] = v, right[v] = best;
			dl[best] += 2;
			if (added == deficit) return added;
		}
		if (++steps == n) steps = 0;
	}
}

inline int augment(const CSR& graph, std::vector<int>& left, std::vector<int>& right,
	int multiplier = 16, int root_divisor = 32, int easy_phases = 8) {
	int roots = 0, n = int(left.size());
	for (int u = 0; u < n; ++u) roots += left[u] < 0 && graph.start[u] != graph.start[u + 1];
	Result initial{0, false};
	if (root_divisor > 0 && roots <= std::max(64, n / root_divisor)) initial = hopcroft_karp(graph, left, right, easy_phases);
	if (initial.maximum || std::find(left.begin(), left.end(), -1) == left.end()) return initial.added;
	int added = initial.added + warm_start(graph, left, right, multiplier);
	if (std::find(left.begin(), left.end(), -1) == left.end()) return added;
	return added + hopcroft_karp(graph, left, right).added;
}

struct Analysis {
	int left_size;
	std::vector<int> start, adj, reverse_start, reverse_adj, component, matched_edge;
	std::vector<unsigned char> from_left, to_right;
	Analysis(int l, int r, std::span<const std::pair<int, int>> edges,
		const std::vector<int>& left, const std::vector<int>& right)
		: left_size(l), start(l + r + 1), adj(edges.size()), reverse_start(l + r + 1),
		  reverse_adj(edges.size()), matched_edge(l, -1), from_left(l + r), to_right(l + r) {
		for (int i = 0; i < int(edges.size()); ++i) {
			auto [u, v] = edges[i];
			if (left[u] == v && matched_edge[u] < 0) matched_edge[u] = i;
			v += l;
			if (matched_edge[u] == i) std::swap(u, v);
			++start[u + 1], ++reverse_start[v + 1];
		}
		std::partial_sum(start.begin(), start.end(), start.begin());
		std::partial_sum(reverse_start.begin(), reverse_start.end(), reverse_start.begin());
		auto next = start, reverse_next = reverse_start;
		for (int i = 0; i < int(edges.size()); ++i) {
			auto [u, v] = edges[i];
			v += l;
			if (matched_edge[u] == i) std::swap(u, v);
			adj[next[u]++] = v, reverse_adj[reverse_next[v]++] = u;
		}
		std::vector<int> queue(l + r);
		int tail = 0;
		for (int u = 0; u < l; ++u) if (left[u] < 0) queue[tail++] = u, from_left[u] = 1;
		reach(queue, tail, start, adj, from_left);
		tail = 0;
		for (int v = 0; v < r; ++v) if (right[v] < 0) queue[tail++] = l + v, to_right[l + v] = 1;
		reach(queue, tail, reverse_start, reverse_adj, to_right);
	}
	static void reach(std::vector<int>& queue, int tail, const std::vector<int>& start,
		const std::vector<int>& adj, std::vector<unsigned char>& seen) {
		for (int head = 0; head < tail; ++head) {
			int u = queue[head];
			for (int e = start[u]; e < start[u + 1]; ++e) if (!seen[adj[e]]) {
				int v = adj[e];
				seen[v] = 1, queue[tail++] = v;
			}
		}
	}
	void components() {
		int n = int(from_left.size());
		if (!component.empty() || !n) return;
		std::vector<unsigned char> seen(n);
		std::vector<int> order, stack, next = start;
		order.reserve(n), stack.reserve(n);
		for (int root = 0; root < n; ++root) if (!seen[root]) {
			seen[root] = 1;
			stack.push_back(root);
			while (!stack.empty()) {
				int u = stack.back();
				if (next[u] == start[u + 1]) order.push_back(u), stack.pop_back();
				else {
					int v = adj[next[u]++];
					if (!seen[v]) seen[v] = 1, stack.push_back(v);
				}
			}
		}
		component.assign(n, -1);
		int count = 0;
		for (auto it = order.rbegin(); it != order.rend(); ++it) if (component[*it] < 0) {
			component[*it] = count;
			stack.push_back(*it);
			while (!stack.empty()) {
				int u = stack.back();
				stack.pop_back();
				for (int e = reverse_start[u]; e < reverse_start[u + 1]; ++e) {
					int v = reverse_adj[e];
					if (component[v] < 0) component[v] = count, stack.push_back(v);
				}
			}
			++count;
		}
	}
};
}

#endif
