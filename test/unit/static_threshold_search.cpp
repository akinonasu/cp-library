#include "data_structure/static_threshold_search.hpp"
#include <random>

template<class T, bool Smaller, bool Reverse> void check(const std::vector<T> &a, std::mt19937_64 &rng) {
	StaticThresholdSearch<T, Smaller, Reverse> index(a);
	int n = int(a.size());
	assert(index.size() == n && index.empty() == a.empty());
	for (int q = 0; q < 220; ++q) {
		int l = int(rng() % (n + 1)), r = int(rng() % (n + 1)); if (l > r) std::swap(l, r);
		T x = q % 4 == 0 ? std::numeric_limits<T>::lowest() : q % 4 == 1 ? std::numeric_limits<T>::max() : q % 4 == 2 && n ? a[rng() % n] : T(rng());
		for (bool strict : {false, true}) {
			int expected = n;
			for (int i = l; i < n; ++i) { bool ok = Smaller ? (strict ? a[i] < x : a[i] <= x) : (strict ? a[i] > x : a[i] >= x); if (ok) { expected = i; break; } }
			if constexpr (!Reverse) {
				assert((strict ? index.next_strict(l, x) : index.next(l, x)) == expected);
				assert((strict ? index.first_strict(l, r, x) : index.first(l, r, x)) == (expected < r ? expected : n));
				if constexpr (Smaller) assert((strict ? index.next_lt(l, x) : index.next_le(l, x)) == expected);
				else assert((strict ? index.next_gt(l, x) : index.next_ge(l, x)) == expected);
			} else {
				expected = n;
				for (int i = 0; i < r; ++i) { bool ok = Smaller ? (strict ? a[i] < x : a[i] <= x) : (strict ? a[i] > x : a[i] >= x); if (ok) expected = i; }
				assert((strict ? index.prev_strict(r, x) : index.prev(r, x)) == expected);
				assert((strict ? index.last_strict(l, r, x) : index.last(l, r, x)) == (expected != n && expected >= l ? expected : n));
				if constexpr (Smaller) assert((strict ? index.prev_lt(r, x) : index.prev_le(r, x)) == expected);
				else assert((strict ? index.prev_gt(r, x) : index.prev_ge(r, x)) == expected);
			}

		}
	}
	auto record = [&](const auto &source, int at) {
		int expected = n;
		if constexpr (Reverse) { for (int i = at; i-- > 0;) if (Smaller ? a[i] < a[at] : a[i] > a[at]) { expected = i; break; } }
		else { for (int i = at + 1; i < n; ++i) if (Smaller ? a[i] < a[at] : a[i] > a[at]) { expected = i; break; } }
		assert(source.next_record(at) == expected);
	};
	if (n <= 129) for (int at = 0; at < n; ++at) record(index, at);
	else for (int at : {0, n / 2, n - 1}) record(index, at);
	auto copy = index; copy.build({T(7)}); assert(index.size() == n);
	copy = index; StaticThresholdSearch<T, Smaller, Reverse> moved(std::move(copy)); assert(copy.empty() && moved.size() == n);
	if (n) for (int at : {0, n / 2, n - 1}) record(moved, at);
	copy = std::move(moved); assert(moved.empty());
	if (n) for (int at : {0, n / 2, n - 1}) record(copy, at);
	copy.clear(); assert(copy.size() == 0);
}

template<class T> void suite() {
	std::mt19937_64 rng(392914);
	for (int n : {0, 1, 2, 31, 32, 63, 64, 65, 127, 128, 129, 255, 4095, 4096, 4097, 10000}) {
		for (int shape = 0; shape < 7; ++shape) {
			std::vector<T> a(n);
			for (int i = 0; i < n; ++i) a[i] = shape == 0 ? T(rng()) : shape == 1 ? T(std::int64_t(i) * i) : shape == 2 ? T(std::int64_t(n - i) * (n - i)) : shape == 3 ? std::numeric_limits<T>::lowest() : shape == 4 ? std::numeric_limits<T>::max() : T(rng() % 10);
			check<T, false, false>(a, rng); check<T, true, false>(a, rng); check<T, false, true>(a, rng); check<T, true, true>(a, rng);
		}
	}
	for (int n : {65, 129, 4097}) for (int at : {0, 31, 32, 63, 64, n / 2, n - 1}) {
		for (bool invert : {false, true}) {
			std::vector<T> a(n, T(invert ? 7 : 0)); a[at] = T(invert ? 0 : 7);
			check<T, false, false>(a, rng); check<T, true, false>(a, rng); check<T, false, true>(a, rng); check<T, true, true>(a, rng);
		}
	}
	for (int trial = 0; trial < 120; ++trial) { std::vector<T> a(rng() % 90); for (auto &x : a) x = T(rng() % 50); check<T, false, false>(a, rng); check<T, true, false>(a, rng); check<T, false, true>(a, rng); check<T, true, true>(a, rng); }
}
int main() { suite<std::int64_t>(); suite<std::uint64_t>(); suite<int>(); suite<unsigned>(); suite<signed char>(); suite<unsigned short>(); }
