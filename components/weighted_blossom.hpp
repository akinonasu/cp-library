#ifndef WEIGHTED_BLOSSOM_HPP
#define WEIGHTED_BLOSSOM_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace weighted_matching_detail {
template <class T> class Blossom {
	struct Edge { int u = 0, v = 0; T weight = 0; };
	int n, stride, nodes;
	std::size_t head = 0;
	std::uint32_t stamp = 0;
	std::vector<Edge> graph;
	std::vector<T> dual;
	std::vector<int> match, base, parent, slack, from, queue, adjacent;
	std::vector<std::size_t> start;
	std::vector<signed char> state;
	std::vector<std::uint32_t> seen;
	std::vector<std::vector<int>> children;
	Edge &edge(int u, int v) { return graph[std::size_t(u) * stride + v]; }
	int &child(int b, int v) { return from[std::size_t(b) * (n + 1) + v]; }
	T gap(const Edge &e) const { return dual[e.u] + dual[e.v] - e.weight * 2; }
	void relax(int u, int b) {
		if (!slack[b] || gap(edge(u, b)) < gap(edge(slack[b], b))) slack[b] = u;
	}
	void reset_slack(int b) {
		slack[b] = 0;
		for (int u = 1; u <= n; ++u)
			if (base[u] != b && state[base[u]] == 0 && edge(u, b).weight > 0) relax(u, b);
	}
	void enqueue(int b) {
		if (b <= n) queue.push_back(b);
		else for (int c : children[b]) enqueue(c);
	}
	void set_base(int b, int root) {
		base[b] = root;
		if (b > n) for (int c : children[b]) set_base(c, root);
	}
	int orient(int b, int c) {
		auto &list = children[b];
		int p = std::find(list.begin(), list.end(), c) - list.begin();
		assert(p < int(list.size()));
		if (p & 1) { std::reverse(list.begin() + 1, list.end()); p = int(list.size()) - p; }
		return p;
	}
	void set_match(int b, int v) {
		Edge e = edge(b, v);
		match[b] = e.v;
		if (b <= n) return;
		int c = child(b, e.u), p = orient(b, c);
		auto &list = children[b];
		for (int i = 0; i < p; ++i) set_match(list[i], list[i ^ 1]);
		set_match(c, v);
		std::rotate(list.begin(), list.begin() + p, list.end());
	}
	void augment(int u, int v) {
		while (u) {
			int next = base[match[u]];
			set_match(u, v);
			if (!next) break;
			u = base[parent[next]];
			set_match(next, u);
			v = next;
		}
	}
	int lca(int u, int v) {
		if (!++stamp) std::fill(seen.begin(), seen.end(), 0), ++stamp;
		for (; u || v; std::swap(u, v)) if (u) {
			if (seen[u] == stamp) return u;
			seen[u] = stamp;
			u = base[match[u]];
			if (u) u = base[parent[u]];
		}
		return 0;
	}
	void contract(int u, int v, int root) {
		int b = n + 1;
		while (b <= nodes && base[b]) ++b;
		nodes = std::max(nodes, b);
		dual[b] = 0; state[b] = 0; match[b] = match[root];
		auto &list = children[b];
		list.clear(); list.push_back(root);
		for (int x = u; x != root;) {
			int y = base[match[x]];
			list.push_back(x); list.push_back(y); enqueue(y);
			x = base[parent[y]];
		}
		std::reverse(list.begin() + 1, list.end());
		for (int x = v; x != root;) {
			int y = base[match[x]];
			list.push_back(x); list.push_back(y); enqueue(y);
			x = base[parent[y]];
		}
		set_base(b, b);
		for (int x = 1; x <= nodes; ++x) edge(b, x) = edge(x, b) = Edge{};
		std::fill_n(from.begin() + std::size_t(b) * (n + 1), n + 1, 0);
		for (int c : list) {
			for (int x = 1; x <= nodes; ++x) if (edge(c, x).weight > 0) {
				if (!edge(b, x).weight || gap(edge(c, x)) < gap(edge(b, x))) {
					edge(b, x) = edge(c, x); edge(x, b) = edge(x, c);
				}
			}
			for (int x = 1; x <= n; ++x) if (child(c, x)) child(b, x) = c;
		}
		reset_slack(b);
	}
	void expand(int b) {
		for (int c : children[b]) set_base(c, c);
		int entry = child(b, edge(b, parent[b]).u), p = orient(b, entry);
		auto &list = children[b];
		for (int i = 0; i < p; i += 2) {
			int u = list[i], v = list[i + 1];
			parent[u] = edge(v, u).u;
			state[u] = 1; state[v] = 0; slack[u] = 0;
			reset_slack(v); enqueue(v);
		}
		state[entry] = 1; parent[entry] = parent[b]; slack[entry] = 0;
		for (int i = p + 1; i < int(list.size()); ++i) {
			state[list[i]] = -1; reset_slack(list[i]);
		}
		base[b] = 0;
	}
	bool found(const Edge &e) {
		int u = base[e.u], v = base[e.v];
		if (state[v] == -1) {
			parent[v] = e.u; state[v] = 1;
			int next = base[match[v]];
			assert(next);
			slack[v] = slack[next] = 0; state[next] = 0; enqueue(next);
		} else if (state[v] == 0) {
			int root = lca(u, v);
			if (!root) { augment(u, v); augment(v, u); return true; }
			contract(u, v, root);
		}
		return false;
	}
	bool step() {
		std::fill(state.begin(), state.end(), -1);
		std::fill(slack.begin(), slack.end(), 0);
		queue.clear(); head = 0;
		for (int b = 1; b <= nodes; ++b) if (base[b] == b && !match[b]) {
			parent[b] = 0; state[b] = 0; enqueue(b);
		}
		if (queue.empty()) return false;
		for (;;) {
			while (head < queue.size()) {
				int u = queue[head++];
				if (state[base[u]] != 0) continue;
				for (std::size_t i = start[u]; i < start[u + 1]; ++i) {
					int v = adjacent[i];
					if (base[u] == base[v]) continue;
					if (!gap(edge(u, v))) { if (found(edge(u, v))) return true; }
					else relax(u, base[v]);
				}
			}
			T delta = std::numeric_limits<T>::max() / 2;
			for (int b = n + 1; b <= nodes; ++b)
				if (base[b] == b && state[b] == 1) delta = std::min(delta, dual[b] / 2);
			for (int b = 1; b <= nodes; ++b) if (base[b] == b && slack[b]) {
				T d = gap(edge(slack[b], b));
				if (state[b] == -1) delta = std::min(delta, d);
				else if (state[b] == 0) delta = std::min(delta, d / 2);
			}
			for (int u = 1; u <= n; ++u) {
				if (state[base[u]] == 0) {
					if (dual[u] <= delta) return false;
					dual[u] -= delta;
				} else if (state[base[u]] == 1) dual[u] += delta;
			}
			for (int b = n + 1; b <= nodes; ++b) if (base[b] == b) {
				if (state[b] == 0) dual[b] += delta * 2;
				else if (state[b] == 1) dual[b] -= delta * 2;
			}
			queue.clear(); head = 0;
			for (int b = 1; b <= nodes; ++b)
				if (base[b] == b && slack[b] && base[slack[b]] != b && !gap(edge(slack[b], b)))
					if (found(edge(slack[b], b))) return true;
			for (int b = n + 1; b <= nodes; ++b)
				if (base[b] == b && state[b] == 1 && !dual[b]) expand(b);
		}
	}
public:
	Blossom(int n, std::span<const std::int64_t> weights, T maximum)
		: n(n), stride(2 * n + 1), nodes(n), graph(std::size_t(stride) * stride), dual(stride),
		match(stride), base(stride), parent(stride), slack(stride), from(std::size_t(stride) * (n + 1)),
		start(n + 2), state(stride, -1), seen(stride), children(stride) {
		for (int u = 1; u <= n; ++u) {
			base[u] = u; child(u, u) = u; dual[u] = maximum;
			start[u] = adjacent.size();
			for (int v = 1; v <= n; ++v) {
				T weight = T(weights[std::size_t(u - 1) * n + v - 1]);
				edge(u, v) = Edge{u, v, weight};
				if (weight > 0) adjacent.push_back(v);
			}
		}
		start[n + 1] = adjacent.size();
		queue.reserve(n);
	}
	void solve(std::vector<int> &mate) {
		while (step()) {}
		for (int u = 1; u <= n; ++u) mate[u - 1] = match[u] ? match[u] - 1 : -1;
	}
};
}

#endif
