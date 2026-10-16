#include "data_structure/segtree_beats.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

template <class T, class Sum>
void check(const SegtreeBeats<T, Sum> &seg, const std::vector<T> &a) {
	assert(seg.size() == int(a.size()));
	assert(seg.values() == a);
	assert(seg.all_sum() == std::accumulate(a.begin(), a.end(), Sum(0)));
	assert(seg.all_min() == (a.empty() ? std::numeric_limits<T>::max() : *std::min_element(a.begin(), a.end())));
	assert(seg.all_max() == (a.empty() ? std::numeric_limits<T>::lowest() : *std::max_element(a.begin(), a.end())));
	for (int i = 0; i < int(a.size()); ++i) assert(seg.get(i) == a[i]);
}

template <class T, class Sum>
void random_test() {
	std::mt19937 rng(72431);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 63, 64, 65, 127, 256, 513}) {
		std::vector<T> a(n);
		for (auto &x : a) x = T(int(rng() % 101) - 50);
		SegtreeBeats<T, Sum> seg(a);
		for (int t = 0; t < 10000; ++t) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			if (t % 17 == 0) l = 0, r = n;
			T x = T(int(rng() % 201) - 100);
			switch (rng() % 10) {
			case 0:
			case 1:
				seg.chmin(l, r, x);
				for (int i = l; i < r; ++i) a[i] = std::min(a[i], x);
				break;
			case 2:
			case 3:
				seg.chmax(l, r, x);
				for (int i = l; i < r; ++i) a[i] = std::max(a[i], x);
				break;
			case 4:
				seg.add(l, r, x);
				for (int i = l; i < r; ++i) a[i] += x;
				break;
			case 5:
				seg.assign(l, r, x);
				std::fill(a.begin() + l, a.begin() + r, x);
				break;
			case 6:
				if (n) seg.set(l % n, x), a[l % n] = x;
				break;
			default:
				assert(seg.sum(l, r) == std::accumulate(a.begin() + l, a.begin() + r, Sum(0)));
				assert(seg.min(l, r) == (l == r ? std::numeric_limits<T>::max() : *std::min_element(a.begin() + l, a.begin() + r)));
				assert(seg.max(l, r) == (l == r ? std::numeric_limits<T>::lowest() : *std::max_element(a.begin() + l, a.begin() + r)));
			}
			if (t % 97 == 0) check(seg, a);
		}
		check(seg, a);
		auto copy = seg;
		copy.assign(0, n, 42);
		check(seg, a);
		check(copy, std::vector<T>(n, 42));
	}
}

int main() {
	random_test<int, long long>();
	random_test<long long, long long>();
	random_test<long long, __int128_t>();
	SegtreeBeats empty;
	check(empty, std::vector<long long>{});
	SegtreeBeats<long long, __int128_t> wide(std::vector<long long>{-4000000000000000000LL, 4000000000000000000LL, 4000000000000000000LL});
	wide.chmin(0, 3, -3000000000000000000LL);
	assert(wide.sum(0, 3) == -__int128_t(1000000000000000000LL) * 10);
	wide.chmax(0, 3, 3000000000000000000LL);
	assert(wide.sum(0, 3) == __int128_t(3000000000000000000LL) * 3);
	for (int n : {1, 2, 15, 1025}) {
		SegtreeBeats seg(n, 7LL);
		seg.add(0, n, 9);
		seg.chmin(0, n, -5);
		seg.chmax(0, n, 11);
		seg.assign(0, n, -2);
		seg.add(0, n, 13);
		check(seg, std::vector<long long>(n, 11));
		std::vector<long long> a(n);
		std::iota(a.begin(), a.end(), 0);
		SegtreeBeats b(a);
		for (int x = n; x >= 0; --x) b.chmin(0, n, x);
		for (int x = 0; x < n; ++x) b.chmax(0, n, x);
		check(b, std::vector<long long>(n, n - 1));
	}
}
