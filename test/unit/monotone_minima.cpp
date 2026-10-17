#include "components/monotone_minima.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>

void require(bool ok) {
	if (!ok) std::abort();
}

void check(const std::vector<std::vector<std::int64_t>> &a, const std::vector<int> &columns) {
	int rows = a.size(), width = columns.size();
	std::vector<int> expected(rows);
	std::uint64_t calls = 0;
	for (int row = 0; row < rows; ++row) {
		int best = 0;
		for (int j = 1; j < width; ++j) if (a[row][j] < a[row][best]) best = j;
		expected[row] = best;
	}
	require(std::is_sorted(expected.begin(), expected.end()));
	auto better = [&](int row, int x, int y) {
		require(0 <= row && row < rows && 0 <= x && x < y && y < width);
		++calls;
		return a[row][y] < a[row][x];
	};
	require(monotone_minima(rows, width, better) == expected);
	require(calls <= std::uint64_t(width) * (std::bit_width(unsigned(rows)) + 1) + rows);
	std::vector<int> inverse(3 * width + 1);
	for (int j = 0; j < width; ++j) inverse[columns[j]] = j;
	auto mapped = monotone_minima(rows, columns, [&](int row, int x, int y) {
		return better(row, inverse[x], inverse[y]);
	});
	for (int row = 0; row < rows; ++row) require(mapped[row] == columns[expected[row]]);
	auto maximum = monotone_minima(rows, width, [&](int row, int x, int y) {
		return -a[row][y] > -a[row][x];
	});
	require(maximum == expected);
}

int main() {
	auto never = [](int, int, int) { std::abort(); return false; };
	require(monotone_minima(0, 0, never).empty());
	require(monotone_minima(0, 123, never).empty());
	require(monotone_minima(0, std::vector<int>{}, never).empty());
	require(monotone_minima(7, 1, never) == std::vector<int>(7, 0));
	require(monotone_minima(7, std::vector<int>{-37}, never) == std::vector<int>(7, -37));
	std::mt19937_64 rng(31674389);
	for (int rows = 1; rows <= 75; ++rows) for (int width = 1; width <= 75; ++width) {
		std::vector<std::vector<std::int64_t>> a(rows, std::vector<std::int64_t>(width));
		std::vector<int> minima(rows), columns(width);
		for (auto &x : minima) x = rng() % width;
		std::sort(minima.begin(), minima.end());
		for (int row = 0; row < rows; ++row) {
			for (int j = 0; j < width; ++j) a[row][j] = 1 + rng() % 1000000;
			for (int j = minima[row]; j < width; ++j) if (rng() % 5 == 0) a[row][j] = 0;
			a[row][minima[row]] = 0;
		}
		for (int j = 0; j < width; ++j) columns[j] = 3 * j + 1;
		std::shuffle(columns.begin(), columns.end(), rng);
		check(a, columns);
	}
	for (int rows = 1; rows <= 4; ++rows) for (int width = 1; width <= 4; ++width) {
		for (unsigned mask = 0; mask < (1U << (rows * width)); ++mask) {
			std::vector<std::vector<std::int64_t>> a(rows, std::vector<std::int64_t>(width));
			std::vector<int> best(rows), columns(width);
			std::iota(columns.begin(), columns.end(), 0);
			for (int i = 0; i < rows; ++i) {
				for (int j = 0; j < width; ++j) a[i][j] = (mask >> (i * width + j)) & 1;
				best[i] = int(std::min_element(a[i].begin(), a[i].end()) - a[i].begin());
			}
			if (std::is_sorted(best.begin(), best.end())) check(a, columns);
		}
	}
	for (int rows : {1, 2, 3, 31, 32, 33, 1023, 1024, 1025, 1000000}) {
		int width = rows == 1 ? 1000000 : 129;
		auto result = monotone_minima(rows, width, [&](int i, int x, int y) {
			auto value = [&](int j) { auto d = std::int64_t(j) - std::int64_t(i) * width / rows; return d * d; };
			return value(y) < value(x);
		});
		for (int i = 0; i < rows; ++i) require(result[i] == std::int64_t(i) * width / rows);
		for (bool rightmost : {false, true}) {
			result = monotone_minima(rows, width, [=](int, int, int) { return rightmost; });
			for (int j : result) require(j == (rightmost ? width - 1 : 0));
		}
	}
	std::vector<int> ids{5, -3, 100, 5};
	auto result = monotone_minima(4, ids, [](int, int, int) { return false; });
	require(result == std::vector<int>(4, 5));
	auto move_only = [ptr = std::make_unique<int>(7)](int, int x, int y) { return std::abs(y - *ptr) < std::abs(x - *ptr); };
	require(monotone_minima(19, 11, std::move(move_only)) == std::vector<int>(19, 7));
}
