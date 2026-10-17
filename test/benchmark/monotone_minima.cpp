#include "components/monotone_minima.hpp"
#include "components/smawk.hpp"
#include "components/fast_io.hpp"
#include <cstdint>
#include <vector>

#ifndef MONOTONE_MINIMA_VARIANT
#define MONOTONE_MINIMA_VARIANT 0
#endif

template <class Better>
std::vector<int> recursive(int rows, int columns, Better better) {
	std::vector<int> best(rows);
	auto solve = [&](auto &&self, int l, int r, int low, int high) -> void {
		if (l == r) return;
		if (low + 1 == high) {
			std::fill(best.begin() + l, best.begin() + r, low);
			return;
		}
		int mid = l + (r - l) / 2, result = low;
		for (int j = low + 1; j < high; ++j) if (better(mid, result, j)) result = j;
		best[mid] = result;
		self(self, l, mid, low, result + 1);
		self(self, mid + 1, r, result, high);
	};
	solve(solve, 0, rows, 0, columns);
	return best;
}

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	std::vector<std::int64_t> a(n), b(m);
	for (auto &x : a) input.read(x);
	for (auto &x : b) input.read(x);
	auto better = [&](int row, int x, int y) {
		if (row < y) return false;
		if (row - x >= n) return true;
		return a[row - y] + b[y] < a[row - x] + b[x];
	};
#if MONOTONE_MINIMA_VARIANT == 0
	auto best = monotone_minima(n + m - 1, m, better);
#elif MONOTONE_MINIMA_VARIANT == 1
	auto best = recursive(n + m - 1, m, better);
#else
	auto best = smawk(n + m - 1, m, better);
#endif
	for (int i = 0; i < n + m - 1; ++i) {
		if (i) output.write(' ');
		output.write(a[i - best[i]] + b[best[i]]);
	}
	output.write('\n');
}
