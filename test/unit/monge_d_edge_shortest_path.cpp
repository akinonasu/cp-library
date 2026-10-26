#include "graph/monge_d_edge_shortest_path.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

using Cost = std::int64_t;
using Wide = __int128_t;
using Matrix = std::vector<std::vector<Cost>>;

std::vector<std::optional<Wide>> brute_force(const Matrix &weight) {
	int n = int(weight.size());
	std::vector<std::optional<Wide>> result(n + 2);
	if (!n) return result;
	std::vector<std::optional<Wide>> previous(n), next(n);
	previous[0] = 0;
	result[0] = previous.back();
	for (int edges = 1; edges <= n + 1; ++edges) {
		std::fill(next.begin(), next.end(), std::nullopt);
		for (int j = 1; j < n; ++j) for (int i = 0; i < j; ++i) if (previous[i]) {
			Wide candidate = *previous[i] + weight[i][j];
			if (!next[j] || candidate < *next[j]) next[j] = candidate;
		}
		result[edges] = next.back();
		previous.swap(next);
	}
	return result;
}

void check(const Matrix &weight) {
	int n = int(weight.size());
	for (int a = 0; a < n; ++a) for (int b = a + 1; b < n; ++b)
		for (int c = b + 1; c < n; ++c) for (int d = c + 1; d < n; ++d)
			assert(Wide(weight[a][c]) + weight[b][d] <= Wide(weight[a][d]) + weight[b][c]);
	auto expected = brute_force(weight);
	for (int d = 0; d <= n + 1; ++d) {
		int calls = 0;
		auto cost = [&](int i, int j) -> Cost {
			assert(0 <= i && i < j && j < n);
			++calls;
			return weight[i][j];
		};
		assert(monge_d_edge_shortest_path<Wide>(n, d, cost) == expected[d]);
		if (!expected[d] || (*expected[d] >= std::numeric_limits<Cost>::min() && *expected[d] <= std::numeric_limits<Cost>::max())) {
			auto actual = monge_d_edge_shortest_path(n, d, cost);
			assert(bool(actual) == bool(expected[d]));
			if (actual) assert(Wide(*actual) == *expected[d]);
		} else {
			bool caught = false;
			try { monge_d_edge_shortest_path(n, d, cost); }
			catch (const std::overflow_error &) { caught = true; }
			assert(caught);
		}
		if (!n || (n == 1 && d == 0) || (n > 1 && d == 0) || d >= n) assert(calls == 0);
	}
}

template<class Weight>
Matrix make_matrix(int n, Weight weight) {
	Matrix result(n, std::vector<Cost>(n));
	for (int i = 0; i < n; ++i) for (int j = i + 1; j < n; ++j) result[i][j] = weight(i, j);
	return result;
}

int main() {
	check({});
	check(Matrix(1, std::vector<Cost>(1)));
	for (int n = 2; n <= 18; ++n) {
		for (Cost constant : {Cost(0), Cost(7), Cost(-11)})
			check(make_matrix(n, [&](int, int) { return constant; }));
		check(make_matrix(n, [](int i, int j) { return Cost(j - i) * (j - i); }));
		check(make_matrix(n, [](int i, int j) { return Cost(j - i) * (j - i) - 31 * Cost(j - i) - 13; }));
		check(make_matrix(n, [](int i, int j) { return Cost(j) * j - Cost(i) * i; }));
	}
	std::mt19937_64 random(9176231);
	for (int trial = 0; trial < 300; ++trial) {
		int n = 2 + random() % 25;
		Matrix weight(n, std::vector<Cost>(n));
		for (int i = 0; i < n; ++i) weight[i][0] = Cost(random() % 2001) - 1000;
		for (int j = 1; j < n; ++j) weight[0][j] = Cost(random() % 2001) - 1000;
		for (int i = 1; i < n; ++i) for (int j = 1; j < n; ++j)
			weight[i][j] = weight[i - 1][j] + weight[i][j - 1] - weight[i - 1][j - 1] - Cost(random() % (trial % 3 ? 30 : 2));
		check(weight);
	}
	for (Cost edge : {std::numeric_limits<Cost>::min(), std::numeric_limits<Cost>::max(), std::numeric_limits<Cost>::min() / 2, std::numeric_limits<Cost>::max() / 2})
		check(make_matrix(7, [&](int, int) { return edge; }));
	for (Cost offset : {std::numeric_limits<Cost>::min(), std::numeric_limits<Cost>::max() - 1000})
		check(make_matrix(9, [&](int i, int j) { return offset + Cost(j - i) * (j - i); }));
	Cost large = std::numeric_limits<Cost>::max();
	Cost even_large = large - 1;
	std::vector<Cost> potential{0, even_large, 0, -even_large, 0, even_large, 0};
	check(make_matrix(int(potential.size()), [&](int i, int j) {
		Wide value = Wide(potential[j]) - potential[i];
		return Cost(value / 2);
	}));
	check(make_matrix(9, [&](int i, int j) {
		Cost row = i % 2 ? large / 2 : -large / 2;
		Cost column = j % 2 ? large / 2 : -large / 2;
		return row + column;
	}));
	assert(monge_d_edge_shortest_path<int>(5, 2, [](int i, int j) { return (j - i) * (j - i); }) == 8);
	assert(monge_d_edge_shortest_path<long long>(5, 3, [](int, int) -> std::int8_t { return -7; }) == -21);
	assert(monge_d_edge_shortest_path<std::int8_t>(5, 2, [](int, int) -> std::int16_t { return 63; }) == 126);
	bool narrow_overflow = false;
	try { monge_d_edge_shortest_path<std::int8_t>(5, 2, [](int, int) { return 64; }); }
	catch (const std::overflow_error &) { narrow_overflow = true; }
	assert(narrow_overflow);
	for (int d : {0, 1, 2, std::numeric_limits<int>::max()}) {
		auto cost = [](int, int) -> Cost { assert(false); return 0; };
		assert(!monge_d_edge_shortest_path(0, d, cost));
		assert(monge_d_edge_shortest_path(1, d, cost) == (d ? std::optional<Cost>{} : std::optional<Cost>{0}));
		if (!d || d >= 2) assert(!monge_d_edge_shortest_path(2, d, cost));
	}
	int n = 4097, d = 137, length = n - 1, small = length / d, extra = length % d;
	auto balanced = monge_d_edge_shortest_path(n, d, [=](int i, int j) -> Cost {
		assert(0 <= i && i < j && j < n);
		return Cost(j - i) * (j - i);
	});
	assert(balanced == Cost(d - extra) * small * small + Cost(extra) * (small + 1) * (small + 1));
}
