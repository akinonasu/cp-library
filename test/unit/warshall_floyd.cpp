#include "graph/warshall_floyd.hpp"
#include <random>
#include <tuple>

template<class T>
std::pair<bool, std::vector<std::vector<T>>> reference(const std::vector<std::vector<T>>& input, T inf) {
	int n = int(input.size());
	const __int128_t infinity = __int128_t(1) << 120;
	std::vector<std::vector<__int128_t>> d(n, std::vector<__int128_t>(n, infinity));
	for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
		d[i][j] = input[i][j] == inf ? infinity : __int128_t(input[i][j]);
		if (i == j) d[i][j] = std::min(d[i][j], __int128_t(0));
	}
	for (int k = 0; k < n; ++k) {
		if (d[k][k] < 0) return {false, {}};
		for (int i = 0; i < n; ++i) if (d[i][k] != infinity)
			for (int j = 0; j < n; ++j) if (d[k][j] != infinity) d[i][j] = std::min(d[i][j], d[i][k] + d[k][j]);
	}
	std::vector<std::vector<T>> result(n, std::vector<T>(n, inf));
	for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
		if (d[i][j] == infinity) continue;
		if (d[i][j] < std::numeric_limits<T>::lowest() || d[i][j] >= inf) throw std::overflow_error("reference");
		result[i][j] = T(d[i][j]);
	}
	return {true, result};
}

template<class T>
void check(const std::vector<std::vector<T>>& input, T inf = std::numeric_limits<T>::max(), bool components = false) {
	auto [ok, expected] = reference(input, inf);
	WarshallFloyd<T> graph(input, inf);
	assert(graph.size() == int(input.size()) && graph.infinity() == inf);
	assert(graph.solve() == ok);
	if (ok) {
		assert(graph.distances() == expected);
		assert(graph.solve());
		assert(graph.distances() == expected);
	}
	auto matrix = input;
	assert(warshall_floyd(matrix, inf) == ok);
	if (ok) assert(matrix == expected);
	if (!components) return;
	constexpr int block = 32;
	int n = int(input.size()), blocks = n / block + (n % block != 0);
	std::size_t padded = std::size_t(blocks) * block;
	warshall_floyd_detail::Storage<T> original(padded * padded, inf);
	if (!original.empty()) assert(reinterpret_cast<std::uintptr_t>(original.data()) % 64 == 0);
	auto index = [&](int i, int j) { return (std::size_t(i / block) * blocks + j / block) * block * block + i % block * block + j % block; };
	for (std::size_t i = 0; i < padded; ++i) original[index(int(i), int(i))] = 0;
	bool negative = false;
	for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
		if (input[i][j] < 0) negative = true;
		if (i == j) original[index(i, j)] = std::min(input[i][j], T(0));
		else original[index(i, j)] = input[i][j];
	}
	for (int backend = 0; backend <= warshall_floyd_detail::backend(); ++backend) {
		auto work = original;
		bool success = negative ? warshall_floyd_detail::run<T, block, true>(work.data(), blocks, inf, backend)
			: warshall_floyd_detail::run<T, block, false>(work.data(), blocks, inf, backend);
		assert(success == ok);
		if (ok) for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) assert(work[index(i, j)] == expected[i][j]);
	}
}

template<class T> void random_cases() {
	std::mt19937_64 rng(20261020);
	const T inf = std::numeric_limits<T>::max();
	for (int n : {0, 1, 2, 3, 7, 15, 31, 32, 33, 63, 64, 65, 127, 129, 255, 257, 511, 512, 513}) {
		std::vector<T> potential(n);
		for (auto& x : potential) x = T(int(rng() % 2001) - 1000);
		for (int shape = 0; shape < 3; ++shape) {
			std::vector<std::vector<T>> d(n, std::vector<T>(n, inf));
			for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
				if (i == j) d[i][j] = 0;
				else if (shape != 2 || (i / 17 == j / 17 && rng() % 4 == 0)) {
					d[i][j] = T(1 + rng() % 10000);
					if (shape != 0) d[i][j] += potential[j] - potential[i];
				}
			}
			check(d, inf, n <= 65 || n == 513);
		}
	}
	for (int n : {513, 1025}) {
		std::vector<std::vector<T>> d(n, std::vector<T>(n, inf));
		for (int i = 0; i < n; ++i) d[i][i] = 0;
		d[0][n - 1] = -3; d[n - 1][0] = 1;
		check(d, inf, true);
	}
	for (int it = 0; it < 150; ++it) {
		int n = 1 + int(rng() % 45);
		std::vector<std::vector<T>> d(n, std::vector<T>(n, inf));
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
			if (rng() % 5 == 0) d[i][j] = T(int(rng() % 201) - 100);
		check(d, inf, true);
	}
}

template<class F> void overflow(F f) {
	bool caught = false;
	try { f(); } catch (const std::overflow_error&) { caught = true; }
	assert(caught);
}

int main() {
	random_cases<std::int32_t>();
	random_cases<std::int64_t>();
	using T = std::int64_t;
	constexpr T inf = std::numeric_limits<T>::max(), low = std::numeric_limits<T>::lowest();
	check<T>({{0, inf, inf}, {inf, 0, -1}, {inf, inf, 0}});
	check<T>({{0, low}, {inf, 0}});
	check<T>({{0, inf - 1}, {inf, 0}});
	check<T>({{0, inf - 7, inf}, {inf, 0, -inf + 10}, {inf, inf, 0}});
	check<T>({{0, inf - 1}, {inf - 1, 0}}, inf, true);
	check<T>({{0, low}, {low, 0}});
	check<T>({{0, 10, 100}, {100, 0, -12}, {100, 100, 0}}, T(1000));
	overflow([&] { auto d = std::vector<std::vector<T>>{{0, low, inf}, {inf, 0, -1}, {inf, inf, 0}}; warshall_floyd(d); });
	overflow([&] { auto d = std::vector<std::vector<T>>{{0, inf - 1, inf}, {inf, 0, 2}, {inf, inf, 0}}; warshall_floyd(d); });
	overflow([] { std::vector<std::vector<int>> d{{0, 60, 100}, {100, 0, 60}, {100, 100, 0}}; warshall_floyd(d, 100); });
	WarshallFloyd<int> g(3);
	g.add_edge(0, 1, 8); g.add_edge(0, 1, 4); g.add_edge(1, 2, -3);
	auto copied = g;
	assert(g.solve() && copied.solve() && g(0, 2) == 1 && copied.distances() == g.distances());
	g.add_edge(2, 0, 0);
	assert(g.solve() && g(1, 0) == -3);
	g.add_edge(2, 0, -2);
	assert(!g.solve());
	bool caught = false;
	try { WarshallFloyd<int> bad(std::vector<std::vector<int>>{{0, 1}, {0}}); } catch (const std::invalid_argument&) { caught = true; }
	assert(caught);
}
