#include "data_structure/point_set_wavelet_matrix.hpp"
#include <cstdlib>
#include <cstdio>
#include <random>

void require(bool value) { if (!value) std::abort(); }

int main() {
	std::mt19937 random(914887);
	for (int n : {0, 1, 2, 3, 7, 8, 31, 32, 33, 63, 64, 65, 127, 257}) {
		std::vector<int> a(n);
		std::vector<std::pair<int, int>> candidates;
		for (int i = 0; i < n; ++i) {
			a[i] = random() % 31;
			for (int x = 0; x < 31; ++x) candidates.emplace_back(i, x);
		}
		PointSetWaveletMatrix<int> wm(a, candidates);
		for (int repeat = 0; repeat < 400; ++repeat) {
			if (n) {
				int p = random() % n, x = random() % 31;
				require(wm.contains(p, x) && wm.set(p, x));
				a[p] = x;
				require(wm[p] == a[p] && wm.set(p, x));
				require(!wm.contains(p, -1) && !wm.set(p, -1) && wm[p] == a[p]);
			}
			int l = random() % (n + 1), r = random() % (n + 1);
			if (l > r) std::swap(l, r);
			std::vector<int> sorted(a.begin() + l, a.begin() + r);
			std::sort(sorted.begin(), sorted.end());
			std::int64_t total = std::accumulate(sorted.begin(), sorted.end(), std::int64_t(0));
			require(wm.range_sum(l, r) == total);
			int x = random() % 31, y = random() % 31;
			if (x > y) std::swap(x, y);
			std::int64_t below = 0, sum = 0;
			int count = 0;
			for (int z : sorted) {
				if (z < y) below += z;
				if (x <= z && z < y) sum += z, ++count;
			}
			require(wm.count(l, r, x) == std::count(sorted.begin(), sorted.end(), x));
			require(wm.range_freq(l, r, x, y) == count && wm.range_sum(l, r, x, y) == sum);
			require(wm.range_sum(l, r, y) == below);
			int k = random() % (sorted.size() + 1);
			std::int64_t small = std::accumulate(sorted.begin(), sorted.begin() + k, std::int64_t(0));
			std::int64_t large = std::accumulate(sorted.end() - k, sorted.end(), std::int64_t(0));
			require(wm.sum_smallest(l, r, k) == small && wm.sum_largest(l, r, k) == large);
			if (k < int(sorted.size())) require(wm.kth_smallest(l, r, k) == sorted[k] && wm.kth_largest(l, r, k) == sorted[sorted.size() - 1 - k]);
			std::int64_t target = repeat % 2 ? large : std::int64_t(random() % (total + 2));
			int lower_small = -1, lower_large = -1, upper_small = -1, upper_large = -1;
			small = large = 0;
			for (int j = 0; j <= int(sorted.size()); ++j) {
				if (small >= target && lower_small == -1) lower_small = j;
				if (large >= target && lower_large == -1) lower_large = j;
				if (small <= target) upper_small = j;
				if (large <= target) upper_large = j;
				if (j < int(sorted.size())) small += sorted[j], large += sorted[sorted.size() - 1 - j];
			}
			require(wm.min_count_smallest(l, r, target) == lower_small && wm.min_count_largest(l, r, target) == lower_large);
			require(wm.max_count_smallest(l, r, target) == upper_small && wm.max_count_largest(l, r, target) == upper_large);
			if (repeat == 100) wm = PointSetWaveletMatrix<int>(wm);
			if (repeat == 200) wm.build(a, candidates);
		}
	}
	std::vector<std::int64_t> extreme{INT64_MIN, 0, INT64_MAX};
	std::vector<std::pair<int, std::int64_t>> candidates{{0, INT64_MAX}, {2, INT64_MIN}};
	PointSetWaveletMatrix<std::int64_t> wide(extreme, candidates);
	require(wide.range_sum(0, 3) == -1);
	require(wide.set(0, INT64_MAX));
	require(wide.range_sum(0, 3) == __int128_t(INT64_MAX) * 2);
	require(wide.set(2, INT64_MIN) && wide.kth_smallest(0, 3, 0) == INT64_MIN);
	require(wide.range_sum(0, 3) == -1);
	PointSetWaveletMatrix<int> empty;
	require(empty.empty() && empty.range_sum(0, 0) == 0 && empty.min_count_largest(0, 0, 1) == -1);
	std::puts("Point Set Wavelet Matrix OK");
}
