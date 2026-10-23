#include "graph/shortest_path/dijkstra.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>
#include <vector>

using namespace std;

template<class T>
void check_path(const vector<vector<pair<int, T>>> &g, int source, int target, T expected, T inf) {
	auto [distance, path] = dijkstra_path(g, source, target, inf);
	assert(distance == expected);
	if (expected == T(-1)) {
		assert(path.empty());
		return;
	}
	assert(!path.empty() && path.front() == source && path.back() == target);
	assert(path.size() <= g.size());
	T sum = 0;
	for (size_t i = 1; i < path.size(); ++i) {
		T weight = inf;
		for (const auto &[to, cost] : g[path[i - 1]])
			if (to == path[i]) weight = min(weight, cost);
		assert(weight < inf && weight < inf - sum);
		sum += weight;
	}
	assert(sum == expected);
}

int main() {
	using I = int64_t;
	using G = vector<vector<pair<int, I>>>;
	mt19937_64 random(123);
	for (int trial = 0; trial < 500; ++trial) {
		int n = 1 + random() % 20, edges = random() % (n * n + 1);
		I inf = trial % 2 ? numeric_limits<I>::max() : DIJKSTRA_INF;
		G g(n);
		vector<vector<I>> expected(n, vector<I>(n, inf));
		for (int i = 0; i < n; ++i) expected[i][i] = 0;
		for (int repeat_index = 0; repeat_index < edges; ++repeat_index) {
			int v = random() % n, to = random() % n;
			I c = trial % 3 == 0 ? random() % 2 : trial % 3 == 1 ? random() % 1000
																 : random() >> 1;
			g[v].emplace_back(to, c);
			expected[v][to] = min(expected[v][to], c);
		}
		for (int k = 0; k < n; ++k)
			for (int i = 0; i < n; ++i)
				for (int j = 0; j < n; ++j) {
					__int128_t sum = __int128_t(expected[i][k]) + expected[k][j];
					if (sum < expected[i][j]) expected[i][j] = I(sum);
				}
		for (int st = 0; st < n; ++st) {
			for (I &v : expected[st])
				if (v == inf) v = -1;
			assert(dijkstra(g, st, inf) == expected[st]);
			for (int target = 0; target < n; ++target)
				check_path(g, st, target, expected[st][target], inf);
		}
	}
	const vector<vector<pair<int, int>>> g = {{{1, 7}}, {}, {}};
	assert((dijkstra(g, 0) == vector<int>{0, 7, -1}));
	const vector<vector<pair<int, double>>> f = {{{1, 1.5}}, {{2, 2.25}}, {}, {}};
	assert((dijkstra(f, 0, 100.) == vector<double>{0, 1.5, 3.75, -1}));
	assert((dijkstra(f, 0) == vector<double>{0, 1.5, 3.75, -1}));
	assert((dijkstra_path(f, 0, 2) == pair<double, vector<int>>{3.75, {0, 1, 2}}));
	const vector<vector<pair<int, double>>> fractional = {
		{{1, 8.5}, {2, 1.25}, {3, 9.5}}, {{3, 2.5}}, {{1, .5}},
		{{4, 5.75}, {5, 5.5}}, {}, {{4, .5}}
	};
	assert((dijkstra(fractional, 0, 10.) == vector<double>{0, 1.75, 1.25, 4.25, -1, 9.75}));
	for (int target = 0; target < int(fractional.size()); ++target)
		check_path(fractional, 0, target, vector<double>{0, 1.75, 1.25, 4.25, -1, 9.75}[target], 10.);
	using U = uint64_t;
	using UnsignedGraph = vector<vector<pair<int, U>>>;
	const U unsigned_inf = numeric_limits<U>::max(), high = (U(1) << 63) + 9;
	const UnsignedGraph unsigned_huge = {
		{{1, high}, {2, high + 100}}, {{2, 7}, {3, unsigned_inf}},
		{{3, unsigned_inf - high - 7}, {4, unsigned_inf - high - 8}},
		{}, {{3, 2}, {5, 0}}, {}, {}
	};
	assert((dijkstra(unsigned_huge, 0, unsigned_inf) ==
		vector<U>{0, high, high + 7, unsigned_inf, unsigned_inf - 1, unsigned_inf - 1, unsigned_inf}));
	for (int target = 0; target < int(unsigned_huge.size()); ++target)
		check_path(unsigned_huge, 0, target,
			vector<U>{0, high, high + 7, unsigned_inf, unsigned_inf - 1, unsigned_inf - 1, unsigned_inf}[target], unsigned_inf);
	const UnsignedGraph unsigned_default = {{{1, U(DIJKSTRA_INF) - 1}, {2, U(DIJKSTRA_INF)}}, {}, {}};
	assert((dijkstra(unsigned_default, 0) == vector<U>{0, U(DIJKSTRA_INF) - 1, unsigned_inf}));
	assert((dijkstra_path(unsigned_default, 0, 2) == pair<U, vector<int>>{unsigned_inf, {}}));
	const vector<vector<pair<int, int8_t>>> narrow = {{{1, 126}, {2, 127}}, {{2, 1}, {3, 0}}, {}, {}};
	assert((dijkstra(narrow, 0) == vector<int8_t>{0, 126, -1, 126}));
	const vector<vector<pair<int, uint16_t>>> narrow_unsigned = {
		{{1, 65534}, {2, 65535}}, {{2, 2}, {3, 0}}, {}, {}
	};
	assert((dijkstra(narrow_unsigned, 0) == vector<uint16_t>{0, 65534, 65535, 65534}));
	G repeated(130);
	for (int v = 1; v < 130; ++v) {
		for (int shift = 45; shift >= 40; --shift) repeated[0].emplace_back(v, (I(1) << shift) + v);
		repeated[v - 1].emplace_back(v, 0);
		repeated[v].emplace_back(v, 0);
		repeated[v].emplace_back(v - 1, 0);
	}
	assert(dijkstra(repeated, 0) == vector<I>(130));
	check_path(repeated, 0, 129, I(0), I(DIJKSTRA_INF));
	const G bounded = {{{1, 5}, {2, 1}, {4, 100}}, {{3, 0}}, {{1, 1}, {3, 9}}, {{1, 0}, {4, 0}}, {}};
	check_path(bounded, 0, 4, I(2), I(DIJKSTRA_INF));
	check_path(bounded, 4, 0, I(-1), I(DIJKSTRA_INF));
	assert((dijkstra_path(bounded, 3, 3) == pair<I, vector<int>>{0, {3}}));
	I maximum = numeric_limits<I>::max();
	const G huge = {{{1, maximum - 1}, {2, maximum}}, {{2, maximum}}, {}};
	assert((dijkstra(huge, 0, maximum) == vector<I>{0, maximum - 1, -1}));
	using Wide = __int128_t;
	using WideGraph = vector<vector<pair<int, Wide>>>;
	for (int trial = 0; trial < 150; ++trial) {
		int n = 1 + random() % 15;
		Wide inf = Wide(1) << 125;
		WideGraph graph(n);
		vector<vector<Wide>> expected(n, vector<Wide>(n, inf));
		for (int i = 0; i < n; ++i) expected[i][i] = 0;
		for (int e = random() % (n * n + 1); e--;) {
			int from = random() % n, to = random() % n;
			Wide weight = (Wide(random() >> 4) << 64) | random();
			if (trial % 3 == 0) weight = random() % 2;
			graph[from].emplace_back(to, weight);
			expected[from][to] = min(expected[from][to], weight);
		}
		for (int k = 0; k < n; ++k)
			for (int i = 0; i < n; ++i)
				for (int j = 0; j < n; ++j)
					expected[i][j] = min(expected[i][j], expected[i][k] + expected[k][j]);
		for (int source = 0; source < n; ++source) {
			for (auto &value : expected[source]) if (value == inf) value = -1;
			assert(dijkstra(graph, source, inf) == expected[source]);
			for (int target = 0; target < n; ++target)
				check_path(graph, source, target, expected[source][target], inf);
		}
	}
	Wide inf = Wide(1) << 125;
	WideGraph wide = {{{1, inf - 1}, {2, inf}}, {{2, inf}}, {}, {}};
	assert((dijkstra(wide, 0, inf) == vector<Wide>{0, inf - 1, -1, -1}));
	const WideGraph wide_default = {{{1, Wide(DIJKSTRA_INF) - 1}, {2, Wide(DIJKSTRA_INF)}}, {}, {}};
	assert((dijkstra(wide_default, 0) == vector<Wide>{0, Wide(DIJKSTRA_INF) - 1, -1}));
	assert((dijkstra_path(wide_default, 0, 1) == pair<Wide, vector<int>>{Wide(DIJKSTRA_INF) - 1, {0, 1}}));
}
