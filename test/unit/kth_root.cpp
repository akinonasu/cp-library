#include "math/kth_root.hpp"
#include <algorithm>
#include <cfenv>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>

using U = std::uint64_t;

bool reference_leq(U a, U k, U n) {
	U p = 1;
	for (U i = 0; i < k; ++i) {
		if (a && p > n / a) return false;
		p *= a;
	}
	return p <= n;
}

void check(U n, U k, U expected) {
	U actual = kth_root(n, k);
	if (actual != expected) {
		std::cerr << n << ' ' << k << ' ' << expected << ' ' << actual << '\n';
		std::abort();
	}
}

void check(U n, U k) {
	U r = kth_root(n, k);
	bool ok = k == 1 ? r == n : k >= 64 ? r == (n != 0) :
		reference_leq(r, k, n) && !reference_leq(r + 1, k, n);
	if (!ok) {
		std::cerr << n << ' ' << k << ' ' << r << '\n';
		std::abort();
	}
}

int main() {
	constexpr U maximum = std::numeric_limits<U>::max();
	for (U k = 1; k <= 70; ++k) {
		U r = 0;
		for (U n = 0; n <= 10000; ++n) {
			if (reference_leq(r + 1, k, n)) ++r;
			check(n, k, r);
		}
		check(maximum, k);
		check(maximum - 1, k);
		check(U(1) << 63, k);
	}
	std::mt19937_64 rng(64329614);
	for (U r = 1; r <= 2642245; ++r) {
		U cube = r * r * r;
		check(cube - 1, 3, r - 1);
		check(cube, 3, r);
		check(cube + 1, 3);
	}
	for (U k = 2; k <= 63; ++k) {
		U max_root = 1;
		while (reference_leq(max_root * 2, k, maximum)) max_root *= 2;
		U lo = max_root, hi = max_root * 2;
		while (lo + 1 < hi) {
			U mid = (lo + hi) / 2;
			if (reference_leq(mid, k, maximum)) lo = mid;
			else hi = mid;
		}
		max_root = lo;
		for (int i = 0; i < 4000; ++i) {
			U r = i < 100 ? std::min(U(i + 1), max_root) :
				i < 200 ? max_root - std::min(U(i - 100), max_root - 1) : 1 + rng() % max_root;
			U p = 1;
			for (U j = 0; j < k; ++j) p *= r;
			check(p - 1, k, r - 1);
			check(p, k, r);
			check(p + 1, k);
		}
	}
	for (int i = 0; i < 250000; ++i) {
		U n = rng(), k = 1 + rng() % 80;
		check(n, k);
		check(n >> (rng() % 64), k);
	}
	for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
		if (std::fesetround(mode)) continue;
		for (U k = 2; k <= 63; ++k) {
			for (int i = 0; i < 1000; ++i) check(rng(), k);
			check(maximum, k);
		}
		for (U r : {U(1), U(2), U(3), U(1) << 26, U(1) << 31, (U(1) << 32) - 1}) {
			check(r * r - 1, 2, r - 1);
			check(r * r, 2, r);
			check(r * r + 1, 2);
		}
	}
	std::fesetround(FE_TONEAREST);
	for (U k : {U(64), U(65), U(1) << 32, maximum}) {
		check(0, k, 0);
		check(1, k, 1);
		check(maximum, k, 1);
	}
}
