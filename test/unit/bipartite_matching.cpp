#include "graph/bipartite_matching.hpp"
#include <cstdlib>
#include <functional>
#include <random>
#include <set>
#include <utility>

void require(bool condition) { if (!condition) std::abort(); }

std::vector<unsigned char> covering_oracle(int l, int r, const std::vector<std::pair<int, int>>& edges) {
	std::vector<unsigned char> possible(1u << (l + r));
	std::function<void(int, unsigned, unsigned)> visit = [&](int u, unsigned used, unsigned covered) {
		if (u == l) { possible[covered] = 1; return; }
		visit(u + 1, used, covered);
		for (auto [from, v] : edges) if (from == u && !(used >> v & 1))
			visit(u + 1, used | (1u << v), covered | (1u << u) | (1u << (l + v)));
	};
	visit(0, 0, 0);
	for (int bit = 0; bit < l + r; ++bit)
		for (unsigned mask = 0; mask < possible.size(); ++mask)
			if (!(mask >> bit & 1)) possible[mask] |= possible[mask | (1u << bit)];
	return possible;
}

void validate_covering(int l, int r, const std::vector<std::pair<int, int>>& edges,
	const BipartiteMatching::VertexSet& required, bool feasible,
	const std::optional<std::vector<std::pair<int, int>>>& result) {
	require(bool(result) == feasible);
	if (!result) return;
	std::vector<int> left(l), right(r);
	for (auto [u, v] : *result) {
		require(0 <= u && u < l && 0 <= v && v < r);
		require(!left[u]++ && !right[v]++);
		require(std::find(edges.begin(), edges.end(), std::pair(u, v)) != edges.end());
	}
	for (int u : required.left) require(left[u]);
	for (int v : required.right) require(right[v]);
	if (required.left.empty() && required.right.empty()) require(result->empty());
}

void covering_check(int l, int r, const std::vector<std::pair<int, int>>& edges) {
	auto possible = covering_oracle(l, r, edges);
	for (bool built : {false, true}) {
		BipartiteMatching graph(l, r);
		for (auto [u, v] : edges) graph.add_edge(u, v);
		std::vector<int> left, right;
		if (built) {
			graph.edge_classification();
			left = graph.left_mates(), right = graph.right_mates();
		}
		auto before = graph;
		const auto& query = graph;
		for (unsigned mask = 0; mask < possible.size(); ++mask) {
			BipartiteMatching::VertexSet required;
			for (int u = l - 1; u >= 0; --u) if (mask >> u & 1) {
				required.left.push_back(u);
				if ((mask + u) % 3 == 0) required.left.push_back(u);
			}
			for (int v = r - 1; v >= 0; --v) if (mask >> (l + v) & 1) {
				required.right.push_back(v);
				if ((mask + v) % 3 == 0) required.right.push_back(v);
			}
			validate_covering(l, r, edges, required, possible[mask], query.matching_covering(required));
			if (!built) validate_covering(l, r, edges, required, possible[mask],
				bipartite_matching_detail::matching_covering(l, r, edges, required.left, required.right));
			if (built) require(graph.left_mates() == left && graph.right_mates() == right);
		}
		require(graph.maximum_matching() == before.maximum_matching());
		require(graph.edge_classification() == before.edge_classification());
		auto after = graph.minimum_vertex_cover(), expected = before.minimum_vertex_cover();
		require(after.left == expected.left && after.right == expected.right);
	}
}

void covering_state_check() {
	BipartiteMatching graph(2, 1);
	graph.add_edge(1, 0);
	graph.add_edge(0, 0);
	auto before = graph.maximum_matching();
	require((before == std::vector<std::pair<int, int>>{{1, 0}}));
	auto status = graph.edge_classification();
	const auto& query = graph;
	BipartiteMatching::VertexSet required{{0, 0}, {0, 0}};
	auto result = query.matching_covering(required);
	require(result && (*result == std::vector<std::pair<int, int>>{{0, 0}}));
	require(graph.maximum_matching() == before && graph.edge_classification() == status);
	require(!query.matching_covering({{0, 1}, {0}}));
	require(graph.maximum_matching() == before);
	auto copy = graph;
	require(copy.matching_covering(required) == result && copy.maximum_matching() == before);
	graph.clear();
	require(query.matching_covering({}) && query.matching_covering({})->empty());
	require(!query.matching_covering(required));
	require(graph.matching_size() == 0 && copy.maximum_matching() == before);
	graph.add_edge(0, 0);
	auto dirty = graph;
	validate_covering(2, 1, {{0, 0}}, required, true, query.matching_covering(required));
	require(graph.maximum_matching() == dirty.maximum_matching());

	BipartiteMatching path(3, 3);
	path.add_edge(0, 0);
	path.add_edge(0, 1);
	path.add_edge(1, 0);
	auto untouched = path;
	validate_covering(3, 3, {{0, 0}, {0, 1}, {1, 0}}, {{0, 1}, {0, 1}}, true,
		std::as_const(path).matching_covering({{0, 1}, {0, 1}}));
	require(path.maximum_matching() == untouched.maximum_matching());
	path.add_edge(2, 2);
	untouched = path;
	validate_covering(3, 3, {{0, 0}, {0, 1}, {1, 0}, {2, 2}}, {{2}, {2}}, true,
		std::as_const(path).matching_covering({{2}, {2}}));
	require(path.maximum_matching() == untouched.maximum_matching());
}

void covering_tests() {
	for (int l = 0; l <= 3; ++l) for (int r = 0; r <= 3; ++r) {
		for (unsigned mask = 0; mask < (1u << (l * r)); ++mask) {
			std::vector<std::pair<int, int>> edges;
			for (int u = 0; u < l; ++u) for (int v = 0; v < r; ++v)
				if (mask >> (u * r + v) & 1) edges.emplace_back(u, v);
			covering_check(l, r, edges);
		}
	}
	std::mt19937 random(20261115);
	for (int trial = 0; trial < 120; ++trial) {
		int l = random() % 5, r = random() % 5;
		std::vector<std::pair<int, int>> edges;
		if (l && r) for (int m = random() % 18; m--; ) edges.emplace_back(random() % l, random() % r);
		covering_check(l, r, edges);
	}
	covering_state_check();
	constexpr int n = 100000;
	BipartiteMatching chain(n, n + 1);
	BipartiteMatching::VertexSet required;
	for (int u = 0; u < n; ++u) {
		chain.add_edge(u, u + 1), chain.add_edge(u, u);
		required.left.push_back(u);
		if (u + 1 < n) required.right.push_back(u);
	}
	auto result = std::as_const(chain).matching_covering(required);
	std::vector<int> left(n, -1), right(n + 1, -1);
	require(result && result->size() == std::size_t(n));
	for (auto [u, v] : *result) {
		require(0 <= u && u < n && (v == u || v == u + 1));
		require(left[u] < 0 && right[v] < 0);
		left[u] = v, right[v] = u;
	}
	for (int u : required.left) require(left[u] >= 0);
	for (int v : required.right) require(right[v] >= 0);
}

void check(int l, int r, const std::vector<std::pair<int, int>>& edges, bool incremental = false) {
	BipartiteMatching graph(l, r);
	graph.reserve(edges.size());
	int best = -1;
	std::uint64_t total = 0;
	std::vector<std::uint64_t> count(edges.size());
	std::vector<int> chosen, used(r);
	std::function<void(int)> visit = [&](int u) {
		if (u == l) {
			if (int(chosen.size()) > best) best = int(chosen.size()), total = 0, std::fill(count.begin(), count.end(), 0);
			if (int(chosen.size()) == best) {
				++total;
				for (int id : chosen) ++count[id];
			}
			return;
		}
		visit(u + 1);
		for (int i = 0; i < int(edges.size()); ++i) if (edges[i].first == u && !used[edges[i].second]) {
			used[edges[i].second] = 1;
			chosen.push_back(i);
			visit(u + 1);
			chosen.pop_back();
			used[edges[i].second] = 0;
		}
	};
	visit(0);
	{
		bipartite_matching_detail::CSR csr(l, edges, false), reverse(r, csr);
		std::vector<int> left(l, -1), right(r, -1), dl(l), dr(r), queue(l + r);
		int initial = 0;
		for (auto it = edges.rbegin(); it != edges.rend(); ++it) {
			auto [u, v] = *it;
			if (left[u] < 0 && right[v] < 0) left[u] = v, right[v] = u, ++initial;
		}
		int added = bipartite_matching_detail::degree_greedy(csr, reverse, left, right, dl, dr, queue);
		int actual = 0;
		for (int u = 0; u < l; ++u) if (left[u] >= 0) {
			++actual;
			require(right[left[u]] == u && std::find(edges.begin(), edges.end(), std::pair(u, left[u])) != edges.end());
		}
		for (int v = 0; v < r; ++v) if (right[v] >= 0) require(left[right[v]] == v);
		require(added >= 0 && actual == initial + added);
		auto result = bipartite_matching_detail::hopcroft_karp(csr, left, right);
		require(result.maximum && actual + result.added == best);
	}
	if (incremental) for (int budget : {0, 1, 16}) {
		bipartite_matching_detail::CSR csr(l, edges, false);
		std::vector<int> left(l, -1), right(r, -1);
		int added = bipartite_matching_detail::warm_start(csr, left, right, budget);
		if (!budget) require(added == 0);
		int actual = 0;
		for (int u = 0; u < l; ++u) if (left[u] >= 0) {
			++actual;
			require(right[left[u]] == u && std::find(edges.begin(), edges.end(), std::pair(u, left[u])) != edges.end());
		}
		require(actual == added);
		for (int v = 0; v < r; ++v) if (right[v] >= 0) require(left[right[v]] == v);
		auto result = bipartite_matching_detail::hopcroft_karp(csr, left, right);
		require(result.maximum && added + result.added == best);
	}
	for (int i = 0; i < int(edges.size()); ++i) {
		require(graph.add_edge(edges[i].first, edges[i].second) == i);
		require(graph.get_edge(i) == edges[i]);
		if (incremental) graph.minimum_vertex_cover(), graph.edge_classification();
	}
	require(graph.left_size() == l && graph.right_size() == r && graph.edge_count() == int(edges.size()));
	require(graph.build() == best && graph.build() == best);
	require(graph.covers_left() == (best == l) && graph.covers_right() == (best == r));
	auto pairs = graph.edges();
	auto ids = graph.matching_edge_ids();
	require(int(pairs.size()) == best && ids.size() == pairs.size());
	std::vector<int> matched_left(l), matched_right(r);
	for (int i = 0; i < best; ++i) {
		auto [u, v] = pairs[i];
		require(!matched_left[u]++ && !matched_right[v]++ && graph.get_edge(ids[i]) == pairs[i]);
		require(graph.left_mates()[u] == v && graph.right_mates()[v] == u);
	}
	for (int u = 0; u < l; ++u) require((graph.left_mates()[u] >= 0) == bool(matched_left[u]));
	for (int v = 0; v < r; ++v) require((graph.right_mates()[v] >= 0) == bool(matched_right[v]));
	auto cover = graph.minimum_vertex_cover(), independent = graph.maximum_independent_set();
	require(int(cover.left.size() + cover.right.size()) == best);
	std::vector<int> selected_left(l), selected_right(r);
	for (int u : cover.left) require(!selected_left[u]++);
	for (int v : cover.right) require(!selected_right[v]++);
	for (auto [u, v] : edges) require(selected_left[u] || selected_right[v]);
	for (int u : independent.left) require(!selected_left[u]++);
	for (int v : independent.right) require(!selected_right[v]++);
	for (int value : selected_left) require(value == 1);
	for (int value : selected_right) require(value == 1);
	auto edge_cover = graph.minimum_edge_cover();
	std::vector<int> degree_left(l), degree_right(r);
	for (auto [u, v] : edges) ++degree_left[u], ++degree_right[v];
	bool isolated = std::find(degree_left.begin(), degree_left.end(), 0) != degree_left.end() ||
		std::find(degree_right.begin(), degree_right.end(), 0) != degree_right.end();
	require(bool(edge_cover) == !isolated);
	if (edge_cover) {
		require(int(edge_cover->size()) == l + r - best);
		require(std::set<int>(edge_cover->begin(), edge_cover->end()).size() == edge_cover->size());
		std::fill(selected_left.begin(), selected_left.end(), 0);
		std::fill(selected_right.begin(), selected_right.end(), 0);
		for (int id : *edge_cover) {
			auto [u, v] = graph.get_edge(id);
			selected_left[u] = selected_right[v] = 1;
		}
		for (int value : selected_left) require(value == 1);
		for (int value : selected_right) require(value == 1);
	}
	for (bool reverse : {false, true}) {
		auto witness = reverse ? graph.hall_witness_right() : graph.hall_witness_left();
		require(bool(witness) == (best < (reverse ? r : l)));
		if (!witness) continue;
		const auto& source = reverse ? witness->right : witness->left;
		const auto& neighbors = reverse ? witness->left : witness->right;
		require(source.size() > neighbors.size());
		std::set<int> s(source.begin(), source.end()), expected, actual(neighbors.begin(), neighbors.end());
		for (auto [u, v] : edges) {
			if (reverse) std::swap(u, v);
			if (s.count(u)) expected.insert(v);
		}
		require(expected == actual);
	}
	auto status = graph.edge_classification();
	require(status == graph.edge_classification());
	for (int i = 0; i < int(edges.size()); ++i) {
		auto expected = count[i] == 0 ? BipartiteMatching::EdgeStatus::never :
			count[i] == total ? BipartiteMatching::EdgeStatus::forced : BipartiteMatching::EdgeStatus::optional;
		require(status[i] == expected);
	}
	graph.clear();
	require(graph.build() == 0 && graph.edge_count() == 0);
	if (l && r) require(graph.add_edge(0, 0) == 0 && graph.build() == 1);
}

int main() {
	covering_tests();
	{
		BipartiteMatching graph(3, 3);
		graph.add_edge(0, 1);
		require(graph.matching_size() == 1);
		graph.add_edge(0, 0);
		graph.add_edge(1, 1);
		auto pairs = graph.maximum_matching();
		require(pairs.size() == 2 && graph.matching_size() == 2);
		require((pairs == std::vector<std::pair<int, int>>{{0, 0}, {1, 1}}));
		graph.add_edge(1, 2);
		graph.add_edge(2, 1);
		require(graph.matching_size() == 3);
		pairs = graph.maximum_matching();
		require((pairs == std::vector<std::pair<int, int>>{{0, 0}, {1, 2}, {2, 1}}));
		require(graph.matching_size() == int(pairs.size()) && graph.edges() == pairs);
		graph.clear();
		require(graph.matching_size() == 0 && graph.maximum_matching().empty());
		BipartiteMatching empty(0, 3);
		require(empty.matching_size() == 0 && empty.maximum_matching().empty());
	}
	for (int l = 0; l <= 4; ++l) for (int r = 0; r <= 4; ++r) if (l * r <= 12) {
		for (int mask = 0; mask < (1 << (l * r)); ++mask) {
			std::vector<std::pair<int, int>> edges;
			for (int u = 0; u < l; ++u) for (int v = 0; v < r; ++v) if (mask >> (u * r + v) & 1) edges.emplace_back(u, v);
			check(l, r, edges);
		}
	}
	std::mt19937 random(20261020);
	for (int trial = 0; trial < 500; ++trial) {
		int l = random() % 8, r = random() % 8;
		std::vector<std::pair<int, int>> edges;
		if (l && r) for (int m = random() % 25; m--; ) edges.emplace_back(random() % l, random() % r);
		check(l, r, edges, true);
	}
	constexpr int n = 200000;
	BipartiteMatching chain(n, n);
	for (int i = 0; i + 1 < n; ++i) chain.add_edge(i, i + 1);
	for (int i = 0; i < n; ++i) chain.add_edge(i, i);
	require(chain.build() == n);
	auto status = chain.edge_classification();
	for (int i = 0; i < n - 1; ++i) require(status[i] == BipartiteMatching::EdgeStatus::never);
	for (int i = n - 1; i < 2 * n - 1; ++i) require(status[i] == BipartiteMatching::EdgeStatus::forced);
	BipartiteMatching unbalanced(200000, 1);
	for (int i = 0; i < 200000; ++i) unbalanced.add_edge(i, 0);
	require(unbalanced.build() == 1);
	for (auto value : unbalanced.edge_classification()) require(value == BipartiteMatching::EdgeStatus::optional);
	for (bool transpose : {false, true}) {
		BipartiteMatching paths(transpose ? n + 1 : n, transpose ? n : n + 1);
		std::vector<std::pair<int, int>> edges;
		for (int u = 0; u < n; ++u) {
			edges.emplace_back(u, u);
			if (u % 100 != 99) edges.emplace_back(u, u + 1);
		}
		std::shuffle(edges.begin(), edges.end(), random);
		for (auto [u, v] : edges) paths.add_edge(transpose ? v : u, transpose ? u : v);
		require(paths.build() == n);
		paths.add_edge(transpose ? n : 0, transpose ? 0 : n);
		require(paths.build() == n && paths.matching_edge_ids().size() == n);
	}
}
