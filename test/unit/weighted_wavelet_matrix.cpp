#include "data_structure/weighted_wavelet_matrix.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>

void require(bool value) { if (!value) std::abort(); }

template <class T>
void check(const std::vector<T> &a, std::mt19937_64 &random) {
	using Sum = __int128_t;
	using WM = WeightedWaveletMatrix<T, std::int64_t, Sum>;
	std::vector<std::int64_t> w(a.size());
	for (auto &x : w) x = random() % 4;
	WM wm(a, w);
	for (int repeat = 0; repeat < 150; ++repeat) {
		if (!a.empty()) {
			int p = random() % a.size();
			w[p] = random() % 5;
			wm.set_weight(p, w[p]);
			require(wm[p] == a[p] && wm.weight(p) == w[p]);
			wm.add_weight(p, 2);
			wm.add_weight(p, -2);
			int to = random() % a.size();
			std::int64_t amount = random() % (w[p] + 1);
			wm.move_weight(p, to, amount);
			w[p] -= amount, w[to] += amount;
			require(wm.weight(p) == w[p] && wm.weight(to) == w[to]);
		}
		int l = random() % (a.size() + 1), r = random() % (a.size() + 1);
		if (l > r) std::swap(l, r);
		std::vector<T> sorted;
		for (int i = l; i < r; ++i) for (int j = 0; j < w[i]; ++j) sorted.push_back(a[i]);
		std::sort(sorted.begin(), sorted.end());
		require(wm.count(l, r) == std::int64_t(sorted.size()));
		Sum total = 0;
		for (auto x : sorted) total += Sum(x);
		require(wm.range_sum(l, r) == total);
		T x = repeat % 3 ? T(random() % 50) : std::numeric_limits<T>::max();
		T y = repeat % 4 ? T(random() % 50) : std::numeric_limits<T>::min();
		if (y < x) std::swap(x, y);
		std::int64_t count = 0, exact = 0;
		Sum sum = 0, sum_below = 0;
		for (auto value : sorted) {
			exact += value == x;
			if (value < y) sum_below += Sum(value);
			if (x <= value && value < y) ++count, sum += Sum(value);
		}
		require(wm.count(l, r, x) == exact);
		require(wm.range_freq(l, r, x, y) == count);
		require(wm.range_freq(l, r, y) == std::lower_bound(sorted.begin(), sorted.end(), y) - sorted.begin());
		require(wm.range_sum(l, r, x, y) == sum && wm.range_sum(l, r, y) == sum_below);
		int k = random() % (sorted.size() + 1);
		Sum small = 0, large = 0;
		for (int i = 0; i < k; ++i) small += Sum(sorted[i]), large += Sum(sorted[sorted.size() - 1 - i]);
		require(wm.sum_smallest(l, r, k) == small && wm.sum_largest(l, r, k) == large);
		if (k < int(sorted.size())) {
			require(wm.kth_smallest(l, r, k) == sorted[k]);
			require(wm.kth_largest(l, r, k) == sorted[sorted.size() - 1 - k]);
		}
		if (a.empty() || *std::min_element(a.begin(), a.end()) >= 0) {
			std::array<Sum, 9> targets{-1, 0, 1, small, large, total, total + 1, total + 13, Sum(1) << 100};
			for (Sum target : targets) {
				std::array<int, 2> lower{-1, -1}, upper{-1, -1};
				std::array<Sum, 2> prefix{};
				for (int j = 0; j <= int(sorted.size()); ++j) {
					for (int direction = 0; direction < 2; ++direction) {
						if (prefix[direction] >= target && lower[direction] == -1) lower[direction] = j;
						if (prefix[direction] <= target) upper[direction] = j;
					}
					if (j < int(sorted.size())) prefix[0] += Sum(sorted[j]), prefix[1] += Sum(sorted[sorted.size() - 1 - j]);
				}
				require(wm.min_count_smallest(l, r, target) == lower[0]);
				require(wm.min_count_largest(l, r, target) == lower[1]);
				require(wm.max_count_smallest(l, r, target) == upper[0]);
				require(wm.max_count_largest(l, r, target) == upper[1]);
			}
		}
		if (repeat == 50) wm = WM(wm);
		if (repeat == 100) wm.build(a, w);
	}
}

int main() {
	std::mt19937_64 random(498937);
	check(std::vector<int>{}, random);
	for (int n : {1, 2, 3, 7, 8, 15, 16, 31, 32, 33, 63, 64, 65, 127, 128, 129, 1025}) {
		std::vector<int> a(n);
		for (auto &x : a) x = random() % 41;
		check(a, random);
		std::sort(a.begin(), a.end());
		check(a, random);
		std::reverse(a.begin(), a.end());
		check(a, random);
		std::fill(a.begin(), a.end(), 0);
		check(a, random);
		std::fill(a.begin(), a.end(), 7);
		check(a, random);
	}
	check(std::vector<std::int64_t>{INT64_MIN, -1, 0, 1, INT64_MAX, INT64_MIN, INT64_MAX}, random);
	check(std::vector<std::uint64_t>{0, 1, UINT64_MAX, UINT64_MAX - 1, std::uint64_t(1) << 63, 0}, random);
	{
		std::vector<std::int64_t> a(2053), w(a.size());
		for (int i = 0; i < int(a.size()); ++i) a[i] = (std::int64_t(i) - 1026) * 1000000000000LL, w[i] = i % 5;
		std::shuffle(a.begin(), a.end(), random);
		WeightedWaveletMatrix<std::int64_t, std::int64_t, __int128_t> wm(a, w, false);
		for (int repeat = 0; repeat < 1000; ++repeat) {
			int p = random() % a.size();
			wm.add_weight(p, 1);
			++w[p];
			int l = random() % a.size(), r = random() % a.size();
			if (l > r) std::swap(l, r);
			++r;
			std::int64_t x = a[random() % a.size()], y = x + std::int64_t(1 + random() % 4) * 1000000000000LL;
			std::int64_t count = 0, exact = 0;
			__int128_t sum = 0;
			for (int i = l; i < r; ++i) {
				if (a[i] == x) exact += w[i];
				if (x <= a[i] && a[i] < y) count += w[i], sum += __int128_t(a[i]) * w[i];
			}
			require(wm.count(l, r, x) == exact && wm.range_freq(l, r, x, y) == count);
			if (repeat == 500) wm.prepare_sum();
			if (repeat >= 500) require(wm.range_sum(l, r, x, y) == sum);
		}
	}
	WeightedWaveletMatrix<int, std::int64_t, __int128_t> counts_only({7, -3, 12, 7}, {4, 2, 0, 1}, false);
	counts_only.add_weight(2, 3);
	counts_only.move_weight(0, 1, 2);
	require(counts_only.count(0, 4) == 10 && counts_only.range_freq(0, 4, 0) == 4);
	require(counts_only.kth_smallest(0, 4, 4) == 7);
	auto copied_counts = counts_only;
	counts_only.prepare_sum();
	require(counts_only.range_sum(0, 4) == 45);
	counts_only.add_weight(1, -1);
	require(counts_only.range_sum(0, 4) == 48);
	copied_counts.prepare_sum();
	require(copied_counts.range_sum(0, 4) == 45);
	copied_counts.build({1, 2, 3}, {1, 2, 3}, false);
	copied_counts.set_weight(1, 4);
	copied_counts.prepare_sum();
	require(copied_counts.range_sum(0, 3) == 18);
	copied_counts.build({}, {}, false);
	copied_counts.prepare_sum();
	require(copied_counts.empty() && copied_counts.count(0, 0) == 0 && copied_counts.range_sum(0, 0) == 0);

	WeightedWaveletMatrix<int, std::int64_t, __int128_t> huge({0, 5, 7}, {1, INT64_MAX / 4, INT64_MAX / 4});
	require(huge.min_count_largest(0, 3, __int128_t(7) * (INT64_MAX / 4)) == INT64_MAX / 4);
	require(huge.max_count_smallest(0, 3, __int128_t(5) * (INT64_MAX / 4)) == INT64_MAX / 4 + 1);
	WeightedWaveletMatrix<int> empty;
	require(empty.empty() && empty.count(0, 0) == 0 && empty.range_sum(0, 0) == 0);
	std::puts("Weighted Wavelet Matrix OK");
}
