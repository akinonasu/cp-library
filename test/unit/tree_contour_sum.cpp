#include "tree/contour_sum.hpp"
#include "math/mint.hpp"
#include <numeric>
#include <random>

void check(int n, const std::vector<std::pair<int, int>>& edges, std::mt19937& rng) {
	std::vector<std::vector<int>> graph(n);
	for (auto [u, v] : edges) graph[u].push_back(v), graph[v].push_back(u);
	std::vector<std::int64_t> values(n);
	for (auto& value : values) value = std::int64_t(rng() % 201) - 100;
	TreeContourSum<> sums(n, edges, values);
	assert(sums.size() == n && sums.empty() == !n);
	if (!n) return;
	std::vector<std::vector<int>> distance(n, std::vector<int>(n, -1));
	for (int root = 0; root < n; ++root) {
		std::vector<int> work{root};
		distance[root][root] = 0;
		for (std::size_t i = 0; i < work.size(); ++i)
			for (int u : graph[work[i]]) if (distance[root][u] == -1) {
				distance[root][u] = distance[root][work[i]] + 1;
				work.push_back(u);
			}
	}
	for (int q = 0; q < 1000; ++q) {
		int v = rng() % n;
		if (q % 3 == 0) {
			std::int64_t x = std::int64_t(rng() % 201) - 100;
			if (q & 1) sums.add(v, x), values[v] += x;
			else sums.set(v, x), values[v] = x;
			assert(sums.get(v) == values[v]);
		} else {
			std::int64_t l = std::int64_t(rng() % (n + 20)) - 10, r = std::int64_t(rng() % (n + 20)) - 10;
			if (q % 10 == 0) l = std::numeric_limits<std::int64_t>::lowest();
			if (q % 11 == 0) r = std::numeric_limits<std::int64_t>::max();
			std::int64_t expected = 0;
			for (int u = 0; u < n; ++u) if (l <= distance[v][u] && distance[v][u] < r) expected += values[u];
			assert(sums.prod(v, l, r) == expected);
		}
	}
	auto copy = sums;
	copy.add(0, 1);
	assert(copy.get(0) == sums.get(0) + 1);
	sums.build(0, {}, {});
	assert(sums.empty());
}

template<class T> void boundary() {
	constexpr int n = 257;
	std::vector<std::pair<int, int>> edges;
	for (int v = 1; v < n; ++v) edges.emplace_back(v - 1, v);
	T maximum = std::numeric_limits<T>::max(), minimum = std::numeric_limits<T>::lowest();
	std::vector<T> values(n);
	values[0] = maximum, values[1] = minimum, values[2] = 11;
	TreeContourSum<T> sums(n, edges, values);
	assert(sums.prod(0, 0, n) == 10 && sums.prod(0, 0, 1) == maximum);
	sums.set(1, maximum);
	assert(sums.get(1) == maximum);
	sums.set(0, minimum);
	assert(sums.prod(1, 0, n) == 10);
	sums.add(2, -11);
	assert(sums.prod(2, 0, n) == -1);
}

int main() {
	std::mt19937 rng(98237);
	TreeContourSum<> empty;
	assert(empty.empty());
	check(0, {}, rng);
	for (int n : {1, 2, 3, 15, 31, 32, 33, 34, 63, 64, 65, 127}) {
		for (int shape = 0; shape < 6; ++shape) {
			std::vector<int> labels(n);
			std::iota(labels.begin(), labels.end(), 0);
			std::shuffle(labels.begin(), labels.end(), rng);
			std::vector<std::pair<int, int>> edges;
			for (int v = 1; v < n; ++v) {
				int p = shape == 0 ? v - 1 : shape == 1 ? 0 : shape == 2 ? (v - 1) / 2 : rng() % v;
				edges.emplace_back(labels[p], labels[v]);
			}
			std::shuffle(edges.begin(), edges.end(), rng);
			check(n, edges, rng);
		}
	}
	boundary<std::int64_t>();
#if defined(__SIZEOF_INT128__)
	boundary<__int128_t>();
#endif
	std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}};
	TreeContourSum<mint> modular(3, edges, {1, 2, 3});
	modular.add(1, -7);
	assert(modular.prod(0, 0, 3) == mint(-1));
	modular.set(1, 10);
	assert(modular.prod(1, 1, 2) == mint(4));
	constexpr int n = 200000;
	edges.clear();
	for (int v = 1; v < n; ++v) edges.emplace_back(v - 1, v);
	TreeContourSum<> path(n, edges, std::vector<std::int64_t>(n, 1));
	for (int q = 0; q < 1000; ++q) {
		int v = rng() % n, l = rng() % n, r = rng() % n;
		if (l > r) std::swap(l, r);
		auto count = [&](int d) { return d <= 0 ? 0 : std::min(n, v + d) - std::max(0, v - d + 1); };
		assert(path.prod(v, l, r) == count(r) - count(l));
	}
	edges.clear();
	for (int v = 1; v < n; ++v) edges.emplace_back(0, v);
	TreeContourSum<> star(n, edges, std::vector<std::int64_t>(n, 1));
	assert(star.prod(0, 1, 2) == n - 1 && star.prod(1, 2, 3) == n - 2);
	star.add(2, 20);
	assert(star.prod(1, 0, n) == n + 20);
}
