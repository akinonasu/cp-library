#include "graph/eulerian_trail.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <utility>
#include <vector>

template<bool Directed>
bool brute(const EulerianTrail<Directed> &graph, int start) {
	int n = graph.size(), m = graph.edge_count();
	if (!n) return true;
	int full = (1 << m) - 1;
	std::vector<unsigned char> reachable((full + 1) * n);
	for (int v = 0; v < n; ++v) if (start == -1 || v == start) reachable[v] = 1;
	for (int mask = 0; mask <= full; ++mask) for (int v = 0; v < n; ++v) {
		if (!reachable[mask * n + v]) continue;
		for (int id = 0; id < m; ++id) if (!(mask >> id & 1)) {
			auto [from, to] = graph.get_edge(id);
			int next = -1;
			if (from == v) next = to;
			else if constexpr (!Directed) { if (to == v) next = from; }
			if (next != -1) reachable[(mask | (1 << id)) * n + next] = 1;
		}
	}
	for (int v = 0; v < n; ++v) if (reachable[full * n + v]) return true;
	return false;
}

template<bool Directed>
void validate(const EulerianTrail<Directed> &graph, int start,
			  const std::optional<typename EulerianTrail<Directed>::Result> &result) {
	assert(result);
	int n = graph.size(), m = graph.edge_count();
	assert(result->edges.size() == std::size_t(m));
	assert(result->vertices.size() == (n ? std::size_t(m) + 1 : 0));
	if (!n) return;
	if (start != -1) assert(result->vertices.front() == start);
	std::vector<bool> used(m);
	for (int v : result->vertices) assert(0 <= v && v < n);
	for (int i = 0; i < m; ++i) {
		int id = result->edges[i], u = result->vertices[i], v = result->vertices[i + 1];
		assert(0 <= id && id < m && !used[id]);
		used[id] = true;
		auto [from, to] = graph.get_edge(id);
		bool valid = from == u && to == v;
		if constexpr (!Directed) valid |= from == v && to == u;
		assert(valid);
	}
}

template<bool Directed>
void check(const EulerianTrail<Directed> &graph) {
	auto saved = graph.edges();
	for (int start = -1; start < graph.size(); ++start) {
		auto result = graph.solve(start);
		assert(bool(result) == brute(graph, start));
		if (result) validate(graph, start, result);
	}
	assert(graph.edges() == saved);
}

template<bool Directed>
void cases() {
	check(EulerianTrail<Directed>());
	for (int n = 1; n <= 3; ++n) {
		std::vector<std::pair<int, int>> choices;
		for (int u = 0; u < n; ++u) for (int v = Directed ? 0 : u; v < n; ++v) choices.emplace_back(u, v);
		for (int m = 0; m <= 4; ++m) {
			std::vector<int> sequence(m);
			while (true) {
				EulerianTrail<Directed> graph(n);
				graph.reserve(m);
				for (int i = 0; i < m; ++i) {
					auto [u, v] = choices[sequence[i]];
					assert(graph.add_edge(u, v) == i);
				}
				check(graph);
				int i = 0;
				while (i < m && ++sequence[i] == int(choices.size())) sequence[i++] = 0;
				if (i == m) break;
			}
		}
	}
	std::mt19937 random(20261021);
	for (int trial = 0; trial < 1000; ++trial) {
		int n = 1 + random() % 6, m = random() % 10;
		std::vector<std::pair<int, int>> edges;
		for (int i = 0; i < m; ++i) edges.emplace_back(random() % n, random() % n);
		EulerianTrail<Directed> graph(n, edges);
		assert(graph.edge_count() == m && graph.edges() == edges);
		check(graph);
		auto first = graph.solve(), second = graph.solve();
		if (first) assert(first->vertices == second->vertices && first->edges == second->edges);
		auto copy = graph;
		copy.clear();
		assert(copy.size() == n && !copy.edge_count());
		validate(copy, -1, copy.solve());
		assert(graph.edges() == edges);
		graph.add_edge(random() % n, random() % n);
		check(graph);
	}
	EulerianTrail<Directed> chain(200000);
	for (int v = 1; v < chain.size(); ++v) chain.add_edge(v - 1, v);
	validate(chain, 0, chain.solve(0));
	if constexpr (Directed) assert(!chain.solve(chain.size() - 1));
	else validate(chain, chain.size() - 1, chain.solve(chain.size() - 1));
	chain.add_edge(chain.size() - 1, 0);
	validate(chain, 12345, chain.solve(12345));
	EulerianTrail<Directed> loops(1);
	loops.reserve(1000000);
	for (int i = 0; i < 1000000; ++i) loops.add_edge(0, 0);
	validate(loops, 0, loops.solve());
}

int main() {
	cases<true>();
	cases<false>();
}
