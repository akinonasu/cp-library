#ifndef MONOTONE_MINIMA_HPP
#define MONOTONE_MINIMA_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace monotone_minima_detail {
template <class Better, class Column>
std::vector<int> solve(int rows, int columns, Better &better, Column column) {
	if (!rows) return {};
	if (columns == 1) return std::vector<int>(rows, column(0));
	std::vector<int> best(std::size_t(rows) + 1);
	best[rows] = columns - 1;
	for (int j = 1; j < columns; ++j)
		if (better(0, column(best[0]), column(j))) best[0] = j;
	for (unsigned step = std::bit_ceil(unsigned(rows)) >> 1; step; step >>= 1) {
		for (unsigned row = step; row < unsigned(rows); row += 2 * step) {
			int lo = best[row - step], hi = best[std::min(row + step, unsigned(rows))];
			int result = lo;
			for (int j = lo + 1; j <= hi; ++j)
				if (better(int(row), column(result), column(j))) result = j;
			best[row] = result;
		}
	}
	best.pop_back();
	for (auto &j : best) j = column(j);
	return best;
}
}

template <class Better>
std::vector<int> monotone_minima(int rows, int columns, Better better) {
	assert(rows >= 0 && columns >= 0 && (!rows || columns));
	return monotone_minima_detail::solve(rows, columns, better, [](int j) { return j; });
}

template <class Better>
std::vector<int> monotone_minima(int rows, const std::vector<int> &columns, Better better) {
	assert(rows >= 0 && columns.size() <= std::size_t(std::numeric_limits<int>::max()) && (!rows || !columns.empty()));
	return monotone_minima_detail::solve(rows, int(columns.size()), better, [&](int j) { return columns[j]; });
}

#endif
