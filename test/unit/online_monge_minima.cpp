#include "components/online_monge_minima.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

using Value = std::int64_t;
using Matrix = std::vector<std::vector<Value>>;

template<class ValueAt>
void check_static(int n, ValueAt value) {
	std::vector<int> actual(n, -1);
	int next = 1;
	online_monge_minima(n,
		[&](int row, int old_col, int new_col) {
			assert(1 <= row && row < n);
			assert(0 <= old_col && old_col < new_col && new_col < row);
			assert(new_col < next);
			assert(!old_col || actual[old_col] >= 0);
			assert(!new_col || actual[new_col] >= 0);
			return value(row, new_col) < value(row, old_col);
		},
		[&](int row, int col) {
			assert(row == next++ && 0 <= col && col < row);
			assert(!col || actual[col] >= 0);
			int best = 0;
			for (int candidate = 1; candidate < row; ++candidate)
				if (value(row, candidate) < value(row, best)) best = candidate;
			assert(col == best);
			actual[row] = col;
		});
	assert(next == std::max(1, n));
}

void check_online(const Matrix &weight, Value penalty) {
	int n = int(weight.size());
	std::vector<Value> actual(n), expected(n);
	std::vector<int> parent(n, -1);
	for (int row = 1; row < n; ++row) {
		expected[row] = std::numeric_limits<Value>::max();
		for (int col = 0; col < row; ++col)
			expected[row] = std::min(expected[row], expected[col] + weight[col][row] + penalty);
	}
	int next = 1;
	auto value = [&](int row, int col) {
		assert(0 <= col && col < row && row < n);
		assert(!col || parent[col] >= 0);
		return actual[col] + weight[col][row] + penalty;
	};
	online_monge_minima(n,
		[&](int row, int old_col, int new_col) {
			assert(0 <= old_col && old_col < new_col && new_col < next);
			return value(row, new_col) < value(row, old_col);
		},
		[&](int row, int col) {
			assert(row == next++);
			int best = 0;
			for (int candidate = 1; candidate < row; ++candidate)
				if (expected[candidate] + weight[candidate][row] < expected[best] + weight[best][row]) best = candidate;
			assert(col == best);
			actual[row] = value(row, col);
			parent[row] = col;
		});
	assert(actual == expected && next == std::max(1, n));
}

int main() {
	for (int n : {0, 1}) {
		online_monge_minima(n,
			[](int, int, int) { assert(false); return false; },
			[](int, int) { assert(false); });
		check_online(Matrix(n, std::vector<Value>(n)), 0);
	}
	for (int n = 2; n <= 80; ++n) {
		check_static(n, [](int, int) -> Value { return 0; });
		check_static(n, [](int, int col) -> Value { return col; });
		check_static(n, [](int, int col) -> Value { return -col; });
		check_static(n, [](int row, int col) -> Value { return Value(col) * col - Value(row) * col; });
		check_static(n, [](int row, int col) -> Value { return Value(col) * col - Value(row / 3) * col; });
		for (int penalty : {-1000, -1, 0, 1, 9, 1000}) {
			Matrix weight(n, std::vector<Value>(n));
			for (int i = 0; i < n; ++i) for (int j = i + 1; j < n; ++j) weight[i][j] = Value(j - i) * (j - i);
			check_online(weight, penalty);
		}
	}
	std::mt19937_64 random(578923041);
	for (int trial = 0; trial < 500; ++trial) {
		int n = 2 + random() % 60;
		Matrix weight(n, std::vector<Value>(n));
		for (int i = 0; i < n; ++i) weight[i][0] = Value(random() % 2001) - 1000;
		for (int j = 1; j < n; ++j) weight[0][j] = Value(random() % 2001) - 1000;
		for (int i = 1; i < n; ++i) for (int j = 1; j < n; ++j)
			weight[i][j] = weight[i - 1][j] + weight[i][j - 1] - weight[i - 1][j - 1] - Value(random() % 15);
		check_static(n, [&](int row, int col) { return weight[col][row]; });
		check_online(weight, Value(random() % 4001) - 2000);
	}
	auto non_monge = [](int row, int col) -> Value {
		Value scale = row % 2 ? 100 : 1;
		return scale * (Value(col) * col - Value(row) * col);
	};
	bool violates_monge = false;
	for (int x = 0; x < 10; ++x) for (int y = x + 1; y < 11; ++y)
		for (int a = y + 1; a < 12; ++a) for (int b = a + 1; b < 13; ++b)
			violates_monge |= non_monge(a, x) + non_monge(b, y) > non_monge(a, y) + non_monge(b, x);
	assert(violates_monge);
	for (int x = 0; x < 35; ++x) for (int y = x + 1; y < 36; ++y) {
		bool seen = false;
		for (int row = y + 1; row < 37; ++row) {
			bool better = non_monge(row, y) < non_monge(row, x);
			assert(!seen || better);
			seen |= better;
		}
	}
	check_static(37, non_monge);
}
