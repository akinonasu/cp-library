#ifndef ONLINE_MONGE_MINIMA_HPP
#define ONLINE_MONGE_MINIMA_HPP

#include <algorithm>
#include <cassert>
#include <vector>

template <class Better, class Set>
void online_monge_minima(int n, Better better, Set set) {
	assert(n >= 0);
	if (n <= 1) return;
	struct Entry { int column, first; };
	std::vector<Entry> envelope(n);
	int begin = 0, end = 1;
	envelope[0] = {0, 1};
	for (int row = 1; row < n; ++row) {
		while (begin + 1 < end && envelope[begin + 1].first <= row) ++begin;
		set(row, envelope[begin].column);
		if (row == n - 1 || !better(n - 1, envelope[end - 1].column, row)) continue;
		while (begin < end && better(std::max(row + 1, envelope[end - 1].first), envelope[end - 1].column, row)) --end;
		int first = row + 1;
		if (begin < end) {
			int low = std::max(row + 1, envelope[end - 1].first), high = n - 1;
			while (high - low > 1) {
				int mid = low + (high - low) / 2;
				if (better(mid, envelope[end - 1].column, row)) high = mid;
				else low = mid;
			}
			first = high;
		}
		envelope[end++] = {row, first};
	}
}

#endif
