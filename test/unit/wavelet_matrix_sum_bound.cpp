#include "data_structure/wavelet_matrix/wavelet_matrix.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>

void require(bool value) { if (!value) std::abort(); }

template <class T>
void check(std::vector<T> a, std::mt19937_64 &random) {
	WaveletMatrix<T> wm(a);
	for (int repeat = 0; repeat < 150; ++repeat) {
		int l = random() % (a.size() + 1), r = random() % (a.size() + 1);
		if (l > r) std::swap(l, r);
		std::vector<T> sorted(a.begin() + l, a.begin() + r);
		std::sort(sorted.begin(), sorted.end());
		using Sum = typename WaveletMatrix<T>::sum_type;
		Sum sum = 0;
		for (auto x : sorted) sum += Sum(x);
		std::array<Sum, 8> targets{-1, 0, 1, sum / 2, sum, sum + 1, sum + 10, std::numeric_limits<Sum>::max()};
		for (Sum target : targets) {
			std::array<int, 2> lower{-1, -1}, upper{-1, -1};
			std::array<Sum, 2> prefix{};
			for (int j = 0; j <= int(sorted.size()); ++j) {
				for (int d = 0; d < 2; ++d) {
					if (prefix[d] >= target && lower[d] == -1) lower[d] = j;
					if (prefix[d] <= target) upper[d] = j;
				}
				if (j < int(sorted.size())) prefix[0] += Sum(sorted[j]), prefix[1] += Sum(sorted[sorted.size() - 1 - j]);
			}
			require(wm.min_count_smallest(l, r, target) == lower[0]);
			require(wm.min_count_largest(l, r, target) == lower[1]);
			require(wm.max_count_smallest(l, r, target) == upper[0]);
			require(wm.max_count_largest(l, r, target) == upper[1]);
		}
		if (repeat == 50) wm = WaveletMatrix<T>(a, true);
		if (repeat == 100) wm.build(a);
	}
}

int main() {
	std::mt19937_64 random(529858);
	for (int n : {0, 1, 2, 3, 7, 8, 15, 16, 31, 32, 33, 63, 64, 65, 257}) {
		std::vector<int> a(n);
		for (auto &x : a) x = random() % 100;
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
	check(std::vector<std::int64_t>{0, INT64_MAX, 1, INT64_MAX, 0, 13}, random);
	check(std::vector<std::int64_t>{0, 9, 1, 20, 0, 7}, random);
	check(std::vector<std::uint64_t>{0, std::uint64_t(INT64_MAX) - 10, 1, 3}, random);
	check(std::vector<std::uint64_t>{0, UINT64_MAX, UINT64_MAX - 1, 1, 0}, random);
	std::puts("Wavelet Matrix sum bounds OK");
}
