#include "data_structure/range_query/parallel_binary_search.hpp"
#include "data_structure/union_find/union_find.hpp"
#include <limits>
#include <random>

int main() {
	std::mt19937 rng(829653);
	for (int m : {0, 1, 2, 3, 7, 16, 37, 100}) for (int q : {0, 1, 2, 17, 300, 3000}) {
		std::vector<int> thresholds(q), expected(q);
		for (int id = 0; id < q; ++id) {
			thresholds[id] = int(rng() % (m + 5)) - 2;
			expected[id] = thresholds[id] <= 0 ? 0 : thresholds[id] > m ? -1 : thresholds[id];
		}
		int time = -1, resets = 0, checks = 0;
		auto result = parallel_binary_search(m, q,
			[&] { time = 0; ++resets; },
			[&](int id) { assert(id == time && id < m); ++time; },
			[&](int id) { assert(0 <= time && time <= m); ++checks; return time >= thresholds[id]; });
		assert(result == expected);
		if (!q) assert(!resets && !checks);
		else if (!m) assert(resets == 1 && checks == q);
	}
	for (int trial = 0; trial < 200; ++trial) {
		int n = 1 + rng() % 20, m = rng() % 60, q = 50;
		std::vector<std::pair<int, int>> edges(m), queries(q);
		for (auto &[a, b] : edges) a = rng() % n, b = rng() % n;
		for (auto &[a, b] : queries) a = rng() % n, b = rng() % n;
		std::vector<int> expected(q, -1);
		std::vector<std::vector<unsigned char>> reach(n, std::vector<unsigned char>(n));
		for (int v = 0; v < n; ++v) reach[v][v] = 1;
		for (int time = 0; time <= m; ++time) {
			if (time) {
				auto [a, b] = edges[time - 1]; reach[a][b] = reach[b][a] = 1;
				for (int k = 0; k < n; ++k) for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v) reach[u][v] |= reach[u][k] && reach[k][v];
			}
			for (int id = 0; id < q; ++id) if (expected[id] < 0 && reach[queries[id].first][queries[id].second]) expected[id] = time;
		}
		UnionFind uf;
		auto run = [&] {
			return parallel_binary_search(m, q, [&] { uf = UnionFind(n); },
				[&](int id) { auto [a, b] = edges[id]; uf.merge(a, b); },
				[&](int id) { auto [a, b] = queries[id]; return uf.same(a, b); });
		};
		assert(run() == expected && run() == expected);
	}
	for (int trial = 0; trial < 100; ++trial) {
		int m = rng() % 100, q = 100;
		std::vector<int> updates(m), prefix(m + 1);
		for (int i = 0; i < m; ++i) prefix[i + 1] = prefix[i] + (updates[i] = rng() % 10);
		std::vector<int> limits(q), expected(q, -1);
		for (int id = 0; id < q; ++id) {
			limits[id] = int(rng() % 1000) - 50;
			for (int t = 0; t <= m; ++t) if (prefix[t] >= limits[id]) { expected[id] = t; break; }
		}
		int sum = 0;
		assert(parallel_binary_search(m, q, [&] { sum = 0; }, [&](int id) { sum += updates[id]; }, [&](int id) { return sum >= limits[id]; }) == expected);
	}
	int resets = 0;
	auto all_true = parallel_binary_search(std::numeric_limits<int>::max(), 5,
		[&] { ++resets; }, [](int) { assert(false); }, [](int) { return true; });
	assert(all_true == std::vector<int>(5, 0) && resets == 1);
	int applied = 0;
	auto all_false = parallel_binary_search(20, 5, [&] { applied = 0; ++resets; },
		[&](int i) { assert(i == applied++); }, [](int) { return false; });
	assert(all_false == std::vector<int>(5, -1) && applied == 20 && resets == 2);
}
