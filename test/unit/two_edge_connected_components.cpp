#include "graph/two_edge_connected_components.hpp"
#include <numeric>
#include <random>

std::vector<int> labels(int n, const std::vector<std::pair<int, int>> &edges, const std::vector<bool> &skip) {
	std::vector<std::vector<int>> adj(n);
	for (int i = 0; i < int(edges.size()); ++i) if (!skip[i]) {
		auto [u, v] = edges[i];
		adj[u].push_back(v), adj[v].push_back(u);
	}
	std::vector<int> id(n, -1), queue;
	int count = 0;
	for (int root = 0; root < n; ++root) if (id[root] == -1) {
		queue.assign(1, root), id[root] = count++;
		for (std::size_t i = 0; i < queue.size(); ++i) for (int to : adj[queue[i]]) if (id[to] == -1)
			id[to] = id[root], queue.push_back(to);
	}
	return id;
}

int count(const std::vector<int> &id) { return id.empty() ? 0 : *std::max_element(id.begin(), id.end()) + 1; }

void check(int n, const std::vector<std::pair<int, int>> &edges) {
	TwoEdgeConnectedComponents graph(n);
	for (auto [u, v] : edges) {
		int id = graph.add_edge(u, v);
		assert(id == int(graph.edges.size()) - 1);
	}
	graph.build();
	std::vector<bool> bridges(edges.size());
	int connected = count(labels(n, edges, bridges));
	for (int i = 0; i < int(edges.size()); ++i) {
		std::vector<bool> skip(edges.size());
		skip[i] = true;
		bridges[i] = count(labels(n, edges, skip)) > connected;
		assert(graph.is_bridge(i) == bridges[i]);
	}
	auto expected = labels(n, edges, bridges);
	assert(graph.component_count() == count(expected));
	for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v)
		assert(graph.same(u, v) == (expected[u] == expected[v]));
	std::vector<int> seen(n), ids;
	auto groups = graph.groups();
	assert(groups.size() == std::size_t(graph.component_count()));
	for (int c = 0; c < graph.component_count(); ++c) {
		auto members = graph.group(c);
		assert(!members.empty() && std::vector<int>(members.begin(), members.end()) == groups[c]);
		for (int v : members) {
			assert(0 <= v && v < n && graph.component[v] == c);
			++seen[v];
		}
	}
	assert(std::all_of(seen.begin(), seen.end(), [](int x) { return x == 1; }));
	auto forest = graph.forest();
	std::vector<int> edge_seen(edges.size());
	int arcs = 0;
	for (int c = 0; c < graph.component_count(); ++c) for (auto [to, id] : forest[c]) {
		assert(c != to && bridges[id]);
		auto [u, v] = edges[id];
		assert((graph.component[u] == c && graph.component[v] == to) || (graph.component[v] == c && graph.component[u] == to));
		++edge_seen[id], ++arcs;
	}
	for (int id : graph.bridges) {
		assert(bridges[id] && edge_seen[id] == 2);
		ids.push_back(id);
	}
	assert(arcs == 2 * (graph.component_count() - connected));
	assert(ids.size() == std::size_t(std::count(bridges.begin(), bridges.end(), true)));
	std::sort(ids.begin(), ids.end());
	assert(std::adjacent_find(ids.begin(), ids.end()) == ids.end());
	auto component = graph.component, old_bridges = graph.bridges;
	graph.build();
	assert(graph.component == component && graph.bridges == old_bridges && graph.groups() == groups);
}

int main() {
	for (int n = 0; n <= 5; ++n) {
		std::vector<std::pair<int, int>> all;
		for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) all.emplace_back(u, v);
		for (int mask = 0; mask < (1 << all.size()); ++mask) {
			std::vector<std::pair<int, int>> edges;
			for (int i = 0; i < int(all.size()); ++i) if (mask >> i & 1) edges.push_back(all[i]);
			check(n, edges);
		}
	}
	std::mt19937 rng(569313);
	for (int trial = 0; trial < 2000; ++trial) {
		int n = 1 + rng() % 14, m = rng() % 45;
		std::vector<std::pair<int, int>> edges;
		for (int i = 0; i < m; ++i) edges.emplace_back(rng() % n, rng() % n);
		check(n, edges);
	}
	TwoEdgeConnectedComponents path(300000);
	path.edges.reserve(path.n);
	for (int v = 1; v < path.n; ++v) path.add_edge(v - 1, v);
	path.build();
	assert(path.component_count() == path.n && path.bridges.size() == std::size_t(path.n - 1));
	for (int c = 0; c < path.component_count(); ++c) assert(path.group(c).size() == 1);
	path.add_edge(0, path.n - 1);
	path.build();
	assert(path.component_count() == 1 && path.bridges.empty() && path.group(0).size() == std::size_t(path.n));
	TwoEdgeConnectedComponents parallel(2);
	parallel.add_edge(0, 1), parallel.build();
	assert(parallel.is_bridge(0));
	parallel.add_edge(1, 0), parallel.add_edge(1, 1), parallel.build();
	assert(parallel.same(0, 1) && parallel.bridges.empty());
}
