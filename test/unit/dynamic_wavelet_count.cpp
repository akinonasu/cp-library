#include "data_structure/dynamic_wavelet_matrix.hpp"
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>

void require(bool ok) {
	if (!ok) std::abort();
}

template <class WM, class T>
void check(const WM &wm, const std::vector<T> &a, int l, int r, T x) {
	int expected = int(std::count(a.begin() + l, a.begin() + r, x));
	require(wm.count(l, r, x) == expected);
	int via_range = x == std::numeric_limits<T>::max() ? r - l - wm.range_freq(l, r, x) :
		wm.range_freq(l, r, x, T(x + 1));
	require(via_range == expected);
}

template <class T>
void boundaries() {
	using U = std::make_unsigned_t<T>;
	std::mt19937_64 rng(18106471);
	std::vector<T> a(4097);
	for (auto &x : a) x = T(rng());
	a[0] = std::numeric_limits<T>::min();
	a.back() = std::numeric_limits<T>::max();
	DynamicWaveletMatrix<T> wm(a);
	for (int p : {0, 1, 63, 64, 65, 511, 512, 1023, 1024, 2047, 2048, 4095, 4096}) {
		for (int bit = 0; bit < 64; ++bit) {
			T x = T(U(a[p]) ^ (U(1) << bit));
			check(wm, a, p, p + 1, a[p]);
			check(wm, a, p, p + 1, x);
			check(wm, a, 0, int(a.size()), x);
		}
	}
	for (T x : {T(0), T(7), std::numeric_limits<T>::min(), std::numeric_limits<T>::max()}) {
		a.assign(4097, x);
		wm.build(a);
		for (int i = 0; i < 128; ++i) {
			int l = rng() % (a.size() + 1), r = rng() % (a.size() + 1);
			if (l > r) std::swap(l, r);
			check(wm, a, l, r, x);
			check(wm, a, l, r, T(U(x) ^ (U(1) << (i % 64))));
		}
		for (int bit = 0; bit < 64; ++bit) {
			T y = T(U(x) ^ (U(1) << bit));
			wm.set(1024, y), a[1024] = y;
			check(wm, a, 0, int(a.size()), x);
			check(wm, a, 1023, 1026, y);
			wm.insert(1024, y), a.insert(a.begin() + 1024, y);
			check(wm, a, 0, int(a.size()), y);
			require(wm.erase(1024) == y);
			a.erase(a.begin() + 1024);
			wm.set(1024, x), a[1024] = x;
			check(wm, a, 0, int(a.size()), x);
			check(wm, a, 0, int(a.size()), y);
		}
	}
}

int main() {
	for (int n = 0, limit = 1; n <= 7; ++n, limit *= 3) {
		for (int mask = 0; mask < limit; ++mask) {
			std::vector<int> a(n);
			for (int i = 0, code = mask; i < n; ++i, code /= 3) a[i] = code % 3 - 1;
			DynamicWaveletMatrix<int, 3> wm(a);
			for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r)
				for (int x = -5; x <= 4; ++x) check(wm, a, l, r, x);
		}
	}
	boundaries<std::int64_t>();
	boundaries<std::uint64_t>();
}
