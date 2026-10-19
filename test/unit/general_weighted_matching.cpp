#include "graph/matching/general_weighted_matching.hpp"
#include <bit>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <random>

using Wide = __int128_t;
using Weight = std::int64_t;

void require(bool ok, int line = __builtin_LINE()) {
	if (!ok) { std::fprintf(stderr, "line %d\n", line); std::abort(); }
}

Wide brute(const std::vector<Weight> &weights, int n) {
	std::vector<Wide> dp(std::size_t(1) << n, -1);
	dp[0] = 0;
	auto solve = [&](auto &&self, unsigned mask) -> Wide {
		if (dp[mask] != -1) return dp[mask];
		int u = std::countr_zero(mask);
		unsigned rest = mask ^ (1U << u);
		Wide best = self(self, rest);
		for (unsigned candidates = rest; candidates; candidates &= candidates - 1) {
			int v = std::countr_zero(candidates);
			if (weights[u * n + v] > 0) best = std::max(best, weights[u * n + v] + self(self, rest ^ (1U << v)));
		}
		return dp[mask] = best;
	};
	return solve(solve, (1U << n) - 1);
}

void check(GeneralWeightedMatching &graph, const std::vector<Weight> &weights, Wide expected) {
	require(graph.build() == expected && graph.weight() == expected);
	int n = graph.size(), count = 0;
	require(int(graph.mate.size()) == n);
	Wide total = 0;
	for (int u = 0; u < n; ++u) if (graph.mate[u] != -1) {
		int v = graph.mate[u];
		require(0 <= v && v < n && u != v && graph.mate[v] == u && weights[u * n + v] > 0);
		if (u < v) { total += weights[u * n + v]; ++count; }
	}
	require(total == expected && graph.matching_size() == count);
	auto pairs = graph.edges();
	require(int(pairs.size()) == count);
	for (auto [u, v] : pairs) require(u < v && graph.mate[u] == v);
	auto mate = graph.mate;
	require(graph.build() == expected && mate == graph.mate);
}

int main() {
	GeneralWeightedMatching empty;
	check(empty, {}, 0); empty.clear(); check(empty, {}, 0);
	for (int n = 0; n <= 4; ++n) {
		int m = n * (n - 1) / 2;
		for (unsigned mask = 0; mask < (1U << (2 * m)); ++mask) {
			GeneralWeightedMatching graph(n);
			std::vector<Weight> weights(n * n);
			int bit = 0;
			for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v, bit += 2) {
				Weight w = (mask >> bit) & 3;
				graph.add_edge(u, v, w); weights[u * n + v] = weights[v * n + u] = w;
			}
			check(graph, weights, brute(weights, n));
		}
	}
	std::mt19937_64 rng(404902);
	for (int n = 1; n <= 18; ++n) for (int trial = 0; trial < 100; ++trial) {
		GeneralWeightedMatching graph(n);
		std::vector<Weight> weights(n * n);
		unsigned density = rng() % 101;
		for (int u = 0; u < n; ++u) {
			graph.add_edge(u, u, LLONG_MAX);
			for (int v = u + 1; v < n; ++v) if (rng() % 100 < density) {
				Weight w = trial % 4 == 0 ? Weight(rng() >> 1) : trial % 4 == 1 ? Weight(rng() % 1000000000000ULL) : Weight(rng() % 21) - 5;
				graph.add_edge(u, v, w);
				weights[u * n + v] = weights[v * n + u] = std::max<Weight>(0, w);
				graph.add_edge(v, u, w); graph.add_edge(u, v, LLONG_MIN);
			}
		}
		Wide expected = brute(weights, n);
		check(graph, weights, expected);
		if (trial % 10 == 0) {
			auto copy = graph;
			check(copy, weights, expected);
			GeneralWeightedMatching moved(std::move(copy));
			check(copy, {}, 0); copy = std::move(moved); check(moved, {}, 0);
			check(copy, weights, expected);
			GeneralWeightedMatching swapped; copy.swap(swapped); swapped.swap(copy);
			for (int u = 1; u < n; ++u) {
				Weight w = Weight(rng() >> 1);
				copy.add_edge(u - 1, u, w);
				weights[(u - 1) * n + u] = weights[u * n + u - 1] = std::max(weights[u * n + u - 1], w);
				check(copy, weights, brute(weights, n));
			}
			copy.clear(); require(copy.size() == n); check(copy, std::vector<Weight>(n * n), 0);
		}
	}
	GeneralWeightedMatching cardinality(4);
	cardinality.add_edge(0, 1, 1); cardinality.add_edge(1, 2, 3); cardinality.add_edge(2, 3, 1);
	require(cardinality.build() == 3 && cardinality.matching_size() == 1);
	GeneralWeightedMatching pending(2);
	pending.add_edge(0, 1, LLONG_MAX);
	GeneralWeightedMatching moved(std::move(pending)); check(pending, {}, 0);
	check(moved, {0, LLONG_MAX, LLONG_MAX, 0}, LLONG_MAX);
	for (Weight w : {Weight(1), Weight(INT_MAX / 404), Weight(INT_MAX / 404) + 1, Weight(LLONG_MAX / 404), Weight(LLONG_MAX / 404) + 1, Weight(LLONG_MAX)}) {
		GeneralWeightedMatching graph(100);
		std::vector<Weight> weights(10000);
		for (int u = 0; u < 100; ++u) for (int v = u + 1; v < 100; ++v) {
			graph.add_edge(u, v, w); weights[u * 100 + v] = weights[v * 100 + u] = w;
		}
		check(graph, weights, Wide(50) * w);
	}
}
