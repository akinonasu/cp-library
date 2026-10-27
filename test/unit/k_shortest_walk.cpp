#include "graph/k_shortest_walk.hpp"
#include "components/directed_csr.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std;

template<class W>
using Graph = vector<vector<pair<int, W>>>;

template<class T, class W>
vector<T> enumerate_walks(const Graph<W> &g, int source, int target, int k, T inf) {
	using U = __uint128_t;
	using State = pair<U, int>;
	priority_queue<State, vector<State>, greater<State>> queue;
	vector<int> count(g.size());
	vector<T> answer;
	queue.emplace(0, source);
	while (!queue.empty() && int(answer.size()) < k) {
		auto [cost, v] = queue.top();
		queue.pop();
		if (count[v] == k) continue;
		++count[v];
		if (v == target) answer.push_back(T(cost));
		for (auto [to, weight] : g[v])
			if (count[to] < k && U(weight) < U(inf) - cost)
				queue.emplace(cost + U(weight), to);
	}
	return answer;
}

template<class W>
DirectedCSR<W> csr(const Graph<W> &g) {
	vector<typename DirectedCSR<W>::Edge> edges;
	for (int v = 0; v < int(g.size()); ++v)
		for (auto [to, weight] : g[v]) edges.push_back({v, to, weight});
	return DirectedCSR<W>(int(g.size()), edges);
}

template<class T = int64_t, class W>
void check(const Graph<W> &g, int source, int target, int k, T inf = numeric_limits<T>::max()) {
	const auto expected = enumerate_walks(g, source, target, k, inf);
	const auto before = g;
	assert(k_shortest_walk<T>(g, source, target, k, inf) == expected);
	assert(g == before);
	assert(k_shortest_walk<T>(csr(g), source, target, k, inf) == expected);
}

int main() {
	using I = int64_t;
	using U = uint64_t;
	using Wide = __int128_t;
	using UWide = __uint128_t;
	static_assert(is_same_v<decltype(k_shortest_walk(Graph<int>{}, 0, 0, 0)), vector<I>>);
	assert(k_shortest_walk(Graph<int>{}, -1, -1, 0).empty());
	assert(k_shortest_walk(DirectedCSR<int>{}, 0, 0, 0).empty());
	assert((k_shortest_walk(Graph<int>(1), 0, 0, 4) == vector<I>{0}));
	assert(k_shortest_walk(Graph<int>(2), 0, 1, 4).empty());
	assert((k_shortest_walk(Graph<int>{{{0, 0}, {0, 0}}}, 0, 0, 40) == vector<I>(40)));
	assert((k_shortest_walk(Graph<int>{{{0, 3}}}, 0, 0, 5) == vector<I>{0, 3, 6, 9, 12}));
	assert((k_shortest_walk(Graph<int>{{{1, 2}, {1, 2}, {1, 5}}, {}}, 0, 1, 5) == vector<I>{2, 2, 5}));
	assert((k_shortest_walk(Graph<int>{{{1, 2}}, {{0, 3}}}, 0, 1, 4) == vector<I>{2, 7, 12, 17}));
	assert((k_shortest_walk(Graph<int>{{{1, 2}}, {{0, 3}}}, 0, 0, 4) == vector<I>{0, 5, 10, 15}));
	Graph<int> zero_cycle{{{1, 3}}, {{2, 0}, {3, 4}}, {{1, 0}}, {}};
	assert(k_shortest_walk(zero_cycle, 0, 3, 50) == vector<I>(50, 7));
	check(zero_cycle, 0, 3, 50);
	check(Graph<int>{{{1, 0}}, {{1, 0}}, {}}, 0, 2, 30);
	check(Graph<int>{{{1, 0}, {2, 3}}, {{1, 0}}, {}}, 0, 2, 30);
	check(Graph<int>{{{1, 5}, {2, 1}}, {{3, 0}}, {{1, 0}, {3, 5}}, {{2, 0}}}, 0, 3, 30);

	mt19937_64 random(7123679);
	for (int trial = 0; trial < 500; ++trial) {
		int n = 1 + random() % 7;
		Graph<U> g(n);
		for (int e = random() % (n * n + 1); e--;) {
			int from = random() % n, to = random() % n;
			U cost = trial % 3 == 0 ? random() % 2 : random() % 30;
			g[from].emplace_back(to, cost);
		}
		int k = random() % 25;
		I inf = trial % 4 == 0 ? 1 + random() % 40 : numeric_limits<I>::max();
		for (int source = 0; source < n; ++source)
			for (int target = 0; target < n; ++target)
				check(g, source, target, k, inf);
	}
	for (int trial = 0; trial < 250; ++trial) {
		int n = 1 + random() % 8;
		Graph<int> g(n);
		for (int v = 0; v < n; ++v)
			for (int to = v + 1; to < n; ++to)
				for (int e = random() % 3; e--;) g[v].emplace_back(to, random() % 7);
		for (int source = 0; source < n; ++source)
			for (int target = 0; target < n; ++target) {
				vector<I> expected;
				auto dfs = [&](auto &&self, int v, I cost) -> void {
					if (v == target) expected.push_back(cost);
					for (auto [to, weight] : g[v]) self(self, to, cost + weight);
				};
				dfs(dfs, source, 0);
				sort(expected.begin(), expected.end());
				int k = trial % 2 ? 2000 : random() % 30;
				if (int(expected.size()) > k) expected.resize(k);
				assert(k_shortest_walk(g, source, target, k) == expected);
			}
	}

	const I imax = numeric_limits<I>::max();
	const U umax = numeric_limits<U>::max();
	const Wide wide_max = numeric_limits<Wide>::max();
	const UWide unsigned_wide_max = numeric_limits<UWide>::max();
	Graph<U> huge{{{1, U(imax) - 3}, {1, U(imax)}, {1, umax}, {2, 1}}, {{1, 1}}, {{1, 2}}};
	check(huge, 0, 1, 30);
	check<U>(huge, 0, 1, 30);
	check<Wide>(huge, 0, 1, 30);
	check<Wide>(huge, 0, 1, 30, Wide(umax) + 5);
	Graph<U> near_unsigned{{{1, umax - 10}, {2, umax - 5}}, {{2, 20}}, {{2, 2}}};
	assert((k_shortest_walk<U>(near_unsigned, 0, 2, 10) == vector<U>{umax - 5, umax - 3, umax - 1}));
	check<U>(near_unsigned, 0, 2, 10);
	Graph<Wide> wide_graph{{{1, wide_max - 10}, {2, wide_max - 5}}, {{2, 20}}, {{2, 2}}};
	check<Wide>(wide_graph, 0, 2, 10);
	check<I>(wide_graph, 0, 2, 10);
	Graph<UWide> unsigned_wide_graph{{{1, unsigned_wide_max - 10}, {2, unsigned_wide_max - 5}}, {{2, 20}}, {{2, 2}}};
	check<UWide>(unsigned_wide_graph, 0, 2, 10);
	check<Wide>(unsigned_wide_graph, 0, 2, 10);
	Graph<UWide> wide_weights{{{1, UWide(1) << 100}, {1, 9}, {1, unsigned_wide_max}}, {{1, 1}}};
	check<int>(wide_weights, 0, 1, 10);
	check<U>(wide_weights, 0, 1, 10);
	check<Wide>(wide_weights, 0, 1, 10);
	check<int8_t>(Graph<U>{{{1, 125}, {1, 126}, {1, 127}, {1, 1000}}, {{1, 1}}}, 0, 1, 10);
	check<uint8_t>(Graph<U>{{{1, 253}, {1, 254}, {1, 255}, {1, 1000}}, {{1, 1}}}, 0, 1, 10);
	check<uint16_t>(Graph<I>{{{1, 65533}, {1, 65534}, {1, 65535}}, {{1, 1}}}, 0, 1, 10);
	assert((k_shortest_walk(Graph<int>{{{0, 3}}}, 0, 0, 10, I(7)) == vector<I>{0, 3, 6}));
	assert((k_shortest_walk(Graph<int>{{{0, 3}}}, 0, 0, 10, I(6)) == vector<I>{0, 3}));

	for (int trial = 0; trial < 200; ++trial) {
		int n = 1 + random() % 6;
		Graph<U> g(n);
		for (int e = random() % (n * n + 1); e--;) {
			U weight = random();
			if (trial % 3 == 0) weight >>= 1;
			if (trial % 7 == 0) weight = random() % 2;
			g[random() % n].emplace_back(random() % n, weight);
		}
		int source = random() % n, target = random() % n, k = 1 + random() % 30;
		check(g, source, target, k);
		check<U>(g, source, target, k);
		check<Wide>(g, source, target, k);
		check<UWide>(g, source, target, k);
	}
	Graph<int> parallel(2);
	for (int i = 4096; i--;) parallel[0].emplace_back(1, i / 3);
	check(parallel, 0, 1, 5000);
	const int n = 20000;
	Graph<int> chain(n);
	for (int v = 1; v < n; ++v) chain[v - 1].emplace_back(v, 1);
	chain.back().emplace_back(n - 1, 2);
	vector<I> expected(40);
	for (int i = 0; i < 40; ++i) expected[i] = n - 1 + 2 * i;
	assert(k_shortest_walk(chain, 0, n - 1, 40) == expected);
	assert(k_shortest_walk(csr(chain), 0, n - 1, 40) == expected);
}
