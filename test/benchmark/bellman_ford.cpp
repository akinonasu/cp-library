#include "graph/bellman_ford.hpp"
#include <chrono>
#include <deque>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

using Graph = DirectedCSR<long long>;
using Result = BellmanFordResult<long long>;

[[gnu::noinline]] Result baseline(const Graph &g, bool queue, bool slf) {
	int n = g.size();
	long long inf = std::numeric_limits<long long>::max();
	Result result{std::vector<long long>(n, inf), std::vector<int>(n, -1), {},
		std::vector<std::size_t>(n, std::size_t(-1)), {}, inf, 0};
	auto &d = result.distance;
	d[0] = 0;
	result.parent[0] = 0;
	auto relax = [&](int v, std::size_t i) {
		auto [to, w] = g[v][i];
		long long next = negative_path_detail::add(d[v], w);
		if (next >= d[to]) return false;
		d[to] = next;
		result.parent[to] = v;
		result.parent_edge[to] = i;
		return true;
	};
	if (!queue) {
		for (int step = 0; step < n; ++step) {
			bool changed = false;
			for (int v = 0; v < n; ++v) if (d[v] != inf)
				for (std::size_t i = 0; i < g[v].size(); ++i) changed |= relax(v, i);
			if (!changed) return result;
		}
		throw std::runtime_error("Negative cycle");
	}
	std::deque<int> pending{0};
	std::vector<unsigned char> inside(n);
	inside[0] = 1;
	while (!pending.empty()) {
		int v = pending.front();
		pending.pop_front();
		inside[v] = 0;
		for (std::size_t i = 0; i < g[v].size(); ++i) {
			int to = g[v][i].first;
			if (!relax(v, i) || inside[to]) continue;
			inside[to] = 1;
			if (slf && !pending.empty() && d[to] < d[pending.front()]) pending.push_front(to);
			else pending.push_back(to);
		}
	}
	return result;
}

[[gnu::noinline]] Result solve(const Graph &g, const std::string &variant) {
	if (variant == "basic") return baseline(g, false, false);
	if (variant == "fifo") return baseline(g, true, false);
	if (variant == "slf") return baseline(g, true, true);
	if (variant == "gor") return goldberg_radzik<long long>(g, 0);
	if (variant == "scc") return negative_path_detail::solve<true, false, long long>(g, 0);
	if (variant == "scan1") return negative_path_detail::solve<true, 1, long long>(g, 0);
	if (variant == "scan4") return negative_path_detail::solve<true, 4, long long>(g, 0);
	return bellman_ford<long long>(g, 0);
}

int main(int argc, char **argv) {
	if (argc != 3) return 2;
	std::string variant = argv[1], kind = argv[2];
	std::mt19937 rng(20261020);
	int n = kind == "dense" ? 3000 : kind == "dag" ? 6000 : kind == "augmented_gor" ? 4001 : 20000;
	std::vector<Graph::Edge> edges;
	std::vector<long long> potential(n);
	for (auto &p : potential) p = int(rng() % 2000001) - 1000000;
	auto add = [&](int v, int to, long long w) { edges.push_back({v, to, w}); };
	if (kind == "dag") {
		std::vector<int> permutation(n);
		std::iota(permutation.begin(), permutation.end(), 0);
		std::shuffle(permutation.begin() + 1, permutation.end(), rng);
		for (int i = 0; i + 1 < n; ++i) add(permutation[i], permutation[i + 1], -1);
		for (int i = 0; i < 3 * n; ++i) {
			int a = rng() % n, b = rng() % n;
			if (a > b) std::swap(a, b);
			if (a < b) add(permutation[a], permutation[b], 1000000);
		}
	} else if (kind == "augmented_gor") {
		int k = (n - 1) / 2;
		add(0, k, -1);
		add(0, 1, -3 * k);
		for (int i = 1; i + 1 < k; ++i) add(i, i + 1, 1);
		for (int i = 1; i < k; ++i) add(i, k, 2 * (k - i - 1));
		for (int i = k + 1; i < n; ++i) add(k, i, -1);
		for (int i = 1; i < n; ++i) add(i, 0, 10 * k);
		for (int i = 0; i < 4 * n; ++i) add(rng() % n, rng() % n, 10 * k);
	} else if (kind == "components") {
		for (int v = 0; v < n; ++v) {
			int to = v / 4 * 4 + (v + 1) % 4;
			add(v, to, 3 + potential[to] - potential[v]);
			if (v + 4 < n) add(v, v + 4, -1);
		}
	} else {
		int m = kind == "dense" ? 300000 : 100000;
		bool positive = kind == "positive_bridge";
		if (positive) { potential.assign(n, 0); add(0, 1, -1000000); }
		for (int v = positive ? 1 : 0; v < n; ++v) {
			int to = v + 1 < n ? v + 1 : (positive ? 1 : 0);
			add(v, to, 1 + potential[to] - potential[v]);
		}
		for (int i = 0; i < m - n; ++i) {
			int v = positive ? 1 + rng() % (n - 1) : rng() % n;
			int to = positive ? 1 + rng() % (n - 1) : rng() % n;
			add(v, to, rng() % 1001 + potential[to] - potential[v]);
		}
	}
	Graph g(n, edges);
	auto begin = std::chrono::steady_clock::now();
	auto result = solve(g, variant);
	auto end = std::chrono::steady_clock::now();
	if (result.has_negative_cycle()) return 3;
	std::uint64_t checksum = 0;
	for (auto x : result.distance) checksum = checksum * 1000000007 + std::uint64_t(x);
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()
		<< ' ' << checksum << ' ' << g.size() << ' ' << edges.size() << '\n';
}
