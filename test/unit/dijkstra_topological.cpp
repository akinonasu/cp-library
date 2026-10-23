#include "graph/shortest_path/dijkstra.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std;

template<class T>
void check(const vector<vector<pair<int, T>>> &graph, T inf) {
	using Sum = conditional_t<is_floating_point_v<T>, long double, __int128_t>;
	int n = int(graph.size());
	vector<vector<Sum>> expected(n, vector<Sum>(n, Sum(inf)));
	for (int v = 0; v < n; ++v) expected[v][v] = 0;
	for (int v = 0; v < n; ++v)
		for (auto [to, weight] : graph[v]) expected[v][to] = min(expected[v][to], Sum(weight));
	for (int middle = 0; middle < n; ++middle)
		for (int from = 0; from < n; ++from)
			for (int to = 0; to < n; ++to)
				expected[from][to] = min(expected[from][to], expected[from][middle] + expected[middle][to]);
	for (int source = 0; source < n; ++source) {
		auto distance = dijkstra<true>(graph, source, inf);
		for (int target = 0; target < n; ++target) {
			bool reachable = expected[source][target] < Sum(inf);
			T wanted = reachable ? T(expected[source][target]) : T(-1);
			assert(distance[target] == wanted);
			auto [cost, path] = dijkstra_path<true>(graph, source, target, inf);
			assert(cost == wanted);
			if (!reachable) {
				assert(path.empty());
				continue;
			}
			assert(!path.empty() && path.front() == source && path.back() == target);
			assert(int(path.size()) <= n);
			vector<bool> seen(n);
			Sum sum = 0;
			for (size_t i = 0; i < path.size(); ++i) {
				assert(path[i] >= 0 && path[i] < n && !seen[path[i]]);
				seen[path[i]] = true;
				if (i + 1 == path.size()) continue;
				Sum weight = Sum(inf);
				for (auto [to, value] : graph[path[i]]) if (to == path[i + 1]) weight = min(weight, Sum(value));
				assert(weight < Sum(inf));
				sum += weight;
			}
			assert(sum == Sum(cost));
		}
	}
}

template<class T>
void random_check(mt19937_64 &random) {
	using Graph = vector<vector<pair<int, T>>>;
	for (int trial = 0; trial < 100; ++trial) {
		int n = 1 + random() % 12;
		T inf = T(100);
		if constexpr (is_integral_v<T>) if (trial % 2) inf = numeric_limits<T>::max();
		Graph graph(n);
		for (int e = random() % (n * n + 1); e--;) {
			int from = random() % n, to = random() % n;
			if (trial % 3 == 0 && from >= to) continue;
			T weight = T(random() % 80);
			if (trial % 5 == 0) weight = T(random() % 2);
			if constexpr (is_integral_v<T>) {
				if (trial % 7 == 0) weight = T((random() & uint64_t(numeric_limits<T>::max())));
			} else weight /= T(4);
			graph[from].emplace_back(to, weight);
		}
		check(graph, inf);
	}
}

int main() {
	mt19937_64 random(4783721);
	random_check<int8_t>(random);
	random_check<uint8_t>(random);
	random_check<int16_t>(random);
	random_check<uint16_t>(random);
	random_check<int32_t>(random);
	random_check<uint32_t>(random);
	random_check<int64_t>(random);
	random_check<uint64_t>(random);
	random_check<float>(random);
	random_check<double>(random);
	random_check<long double>(random);
	using I = int64_t;
	using Graph = vector<vector<pair<int, I>>>;
	Graph core(105);
	core[0].emplace_back(1, I(1) << 40);
	for (int v = 1; v < 105; ++v) core[v].emplace_back(v, 0);
	for (int v = 2; v < 102; ++v) core[1].emplace_back(v, v);
	core[1].emplace_back(102, I(1) << 42);
	core[102].emplace_back(103, 1);
	core[2].emplace_back(102, 1);
	check(core, numeric_limits<I>::max());
	const Graph mixed = {{{1, 4}, {2, 10}}, {{2, 3}}, {{3, 1}}, {{2, 0}, {4, 2}}, {}};
	check(mixed, DIJKSTRA_INF);
	assert((dijkstra<true>(mixed, 0) == vector<I>{0, 4, 7, 8, 10}));
	assert((dijkstra_path<true>(mixed, 0, 4) == pair<I, vector<int>>{10, {0, 1, 2, 3, 4}}));
	const Graph zero = {{{1, 0}}, {{2, 0}}, {{1, 0}, {3, 0}}, {}, {}};
	check(zero, DIJKSTRA_INF);
	const vector<vector<pair<int, uint64_t>>> huge = {
		{{1, numeric_limits<uint64_t>::max() - 1}, {2, numeric_limits<uint64_t>::max()}},
		{{2, 0}, {3, 1}}, {{3, 2}}, {}
	};
	check(huge, numeric_limits<uint64_t>::max());
	const Graph dag = {{{1, 3}}, {{2, 4}}, {}, {{4, 0}}, {}};
	check(dag, DIJKSTRA_INF);
	using CSR = DirectedCSR<uint32_t>;
	const vector<CSR::Edge> edges = {{0, 1, 3'000'000'000u}, {1, 1, 0}, {1, 2, 3'000'000'000u}, {2, 3, 10}, {3, 2, 0}};
	const CSR csr(5, edges);
	static_assert(is_same_v<decltype(dijkstra<true>(csr, 0)), vector<I>>);
	static_assert(is_same_v<decltype(dijkstra_path<true>(csr, 0, 3)), pair<I, vector<int>>>);
	assert((dijkstra<true>(csr, 0) == vector<I>{0, 3'000'000'000, 6'000'000'000, 6'000'000'010, -1}));
	assert((dijkstra_path<true>(csr, 0, 3) == pair<I, vector<int>>{6'000'000'010, {0, 1, 2, 3}}));
	assert((dijkstra_path<true>(csr, 0, 4) == pair<I, vector<int>>{-1, {}}));
	assert(dijkstra(mixed, 0) == dijkstra<I>(mixed, 0));
	assert(dijkstra(mixed, 0) == dijkstra<true>(mixed, 0));
	assert(dijkstra(csr, 0) == dijkstra<true>(csr, 0));
	const vector<CSR::Edge> clipped_edges = {{0, 1, 4'000'000'000u}, {1, 1, 0}, {0, 2, 7}};
	const CSR clipped(3, clipped_edges);
	assert((dijkstra<true>(clipped, 0, 100) == vector<int>{0, -1, 7}));
	assert((dijkstra_path<true>(clipped, 0, 1, 100) == pair<int, vector<int>>{-1, {}}));
	assert((dijkstra_path<true>(clipped, 0, 2, 100) == pair<int, vector<int>>{7, {0, 2}}));
}
