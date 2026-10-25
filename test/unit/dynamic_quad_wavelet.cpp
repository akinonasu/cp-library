#include "data_structure/dynamic_wavelet_matrix.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

template<class T, int Bits> T from_key(std::make_unsigned_t<T> key) {
	using U = std::make_unsigned_t<T>;
	constexpr int width = std::numeric_limits<U>::digits;
	constexpr U mask = std::numeric_limits<U>::max() >> (width - Bits);
	constexpr U sign = std::is_signed_v<T> ? U(1) << (Bits - 1) : 0;
	key ^= sign;
	if constexpr (std::is_signed_v<T> && Bits < width) if (key & sign) key |= U(~mask);
	return T(key);
}

template<class WM, class T>
void query(const WM& wm, const std::vector<T>& values, int l, int r, T lower, T upper) {
	std::vector<T> sorted(values.begin() + l, values.begin() + r);
	std::sort(sorted.begin(), sorted.end());
	int a = int(std::lower_bound(sorted.begin(), sorted.end(), lower) - sorted.begin());
	assert(wm.range_freq(l, r, lower) == a);
	assert(wm.range_freq(l, r, lower, upper) == std::count_if(sorted.begin(), sorted.end(), [&](T x) { return lower <= x && x < upper; }));
	assert(wm.count(l, r, lower) == std::count(sorted.begin(), sorted.end(), lower));
	assert(wm.prev_value(l, r, lower) == (a ? std::optional<T>(sorted[a - 1]) : std::nullopt));
	assert(wm.next_value(l, r, lower) == (a < int(sorted.size()) ? std::optional<T>(sorted[a]) : std::nullopt));
	if (l < r) for (int k : {0, (r - l) / 2, r - l - 1}) {
		assert(wm.kth_smallest(l, r, k) == sorted[k]);
		assert(wm.kth_largest(l, r, k) == sorted[r - l - 1 - k]);
	}
}

template<class WM, class T> void select_check(const WM& wm, const std::vector<T>& values, T value) {
	std::vector<int> positions;
	for (int i = 0; i < int(values.size()); ++i) if (values[i] == value) positions.push_back(i);
	assert(wm.select(value, -1) == -1);
	assert(wm.select(value, int(positions.size())) == -1);
	assert(wm.select(value, int(positions.size()) + 1) == -1);
	assert(wm.rank(value, int(values.size())) == int(positions.size()));
	for (int i = 0; i < int(positions.size()); i += std::max(1, int(positions.size()) / 11)) assert(wm.select(value, i) == positions[i]);
	if (!positions.empty()) assert(wm.select(value, int(positions.size()) - 1) == positions.back());
}

template<class WM, class T> void snapshot(const WM& wm, const std::vector<T>& values, std::mt19937_64& rng) {
	assert(wm.size() == int(values.size()) && wm.empty() == values.empty());
	for (int i = 0; i < int(values.size()); ++i) assert(wm[i] == values[i] && wm.access(i) == values[i]);
	for (int it = 0; it < 50; ++it) {
		int l = int(rng() % (values.size() + 1)), r = int(rng() % (values.size() + 1));
		if (l > r) std::swap(l, r);
		T x = T(rng()), y = T(rng());
		if (!values.empty() && it % 2 == 0) x = values[rng() % values.size()];
		if (it == 0) x = std::numeric_limits<T>::lowest();
		if (it == 1) x = std::numeric_limits<T>::max();
		query(wm, values, l, r, x, y);
		select_check(wm, values, x);
	}
}

template<class T, int Bits> void exhaustive() {
	using U = std::make_unsigned_t<T>;
	constexpr int width = std::numeric_limits<U>::digits;
	constexpr U mask = std::numeric_limits<U>::max() >> (width - Bits);
	std::vector<T> domain{from_key<T, Bits>(0), from_key<T, Bits>(mask / 2), from_key<T, Bits>(mask)};
	std::sort(domain.begin(), domain.end()); domain.erase(std::unique(domain.begin(), domain.end()), domain.end());
	std::vector<T> bounds = domain;
	bounds.push_back(std::numeric_limits<T>::lowest()); bounds.push_back(std::numeric_limits<T>::max());
	for (T x : domain) {
		if (x > std::numeric_limits<T>::lowest()) bounds.push_back(T(x - 1));
		if (x < std::numeric_limits<T>::max()) bounds.push_back(T(x + 1));
	}
	std::sort(bounds.begin(), bounds.end()); bounds.erase(std::unique(bounds.begin(), bounds.end()), bounds.end());
	for (int n = 0, count = 1; n <= 5; ++n, count *= int(domain.size())) for (int code = 0; code < count; ++code) {
		std::vector<T> values(n);
		for (int i = 0, k = code; i < n; ++i, k /= int(domain.size())) values[i] = domain[k % domain.size()];
		DynamicWaveletMatrix<T, Bits> bulk(values), inserted;
		for (int i = n - 1; i >= 0; --i) inserted.insert(0, values[i]);
		for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) for (T x : bounds) for (T y : bounds) {
			query(bulk, values, l, r, x, y);
			query(inserted, values, l, r, x, y);
		}
		for (T x : bounds) { select_check(bulk, values, x); select_check(inserted, values, x); }
	}
}

template<class T, int Bits> void odd_and_full() {
	using U = std::make_unsigned_t<T>;
	constexpr int width = std::numeric_limits<U>::digits;
	constexpr U mask = std::numeric_limits<U>::max() >> (width - Bits);
	std::mt19937_64 rng(7183 + Bits + sizeof(T));
	std::vector<U> keys{0, U(1) & mask, U(2) & mask, U(3) & mask, mask, U(mask - 1)};
	for (int b = 0; b < Bits; ++b) {
		U x = U(1) << b;
		keys.push_back(x); keys.push_back(x - 1);
		if (x < mask) keys.push_back(x + 1);
	}
	std::sort(keys.begin(), keys.end()); keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
	std::vector<T> values;
	for (U key : keys) values.push_back(from_key<T, Bits>(key));
	DynamicWaveletMatrix<T, Bits> bulk(values), inserted;
	for (T x : values) inserted.push_back(x);
	for (T x : values) for (T y : values) {
		query(bulk, values, 0, int(values.size()), x, y);
		query(inserted, values, 0, int(values.size()), x, y);
	}
	for (T x : values) select_check(bulk, values, x);
	for (int n : {0, 1, 2, 63, 64, 65, 127, 128, 129, 511, 513}) {
		values.resize(n);
		for (auto& x : values) x = from_key<T, Bits>(U(rng()) & mask);
		if (n) values[0] = from_key<T, Bits>(0), values[n - 1] = from_key<T, Bits>(mask);
		bulk.build(values); inserted.clear();
		for (T x : values) inserted.push_back(x);
		snapshot(bulk, values, rng); snapshot(inserted, values, rng);
		for (int it = 0; it < 150; ++it) {
			int op = int(rng() % 3), p = int(rng() % (values.size() + 1));
			T x = from_key<T, Bits>(U(rng()) & mask);
			if (values.empty() || op == 0) bulk.insert(p, x), inserted.insert(p, x), values.insert(values.begin() + p, x);
			else {
				p %= int(values.size());
				if (op == 1) { assert(bulk.erase(p) == values[p]); assert(inserted.erase(p) == values[p]); values.erase(values.begin() + p); }
				else bulk.set(p, x), inserted.set(p, x), values[p] = x;
			}
			if (it % 37 == 0) snapshot(bulk, values, rng), snapshot(inserted, values, rng);
		}
		snapshot(bulk, values, rng); snapshot(inserted, values, rng);
	}
}

template<class T, int Bits> void structure() {
	using U = std::make_unsigned_t<T>;
	constexpr U mask = std::numeric_limits<U>::max() >> (std::numeric_limits<U>::digits - Bits);
	std::mt19937_64 rng(197612);
	using WM = dynamic_wavelet_detail::QuadMatrix<T, Bits, 128, 4>;
	for (int mode = 0; mode < 3; ++mode) {
		WM wm;
		wm.reserve(514);
		std::vector<T> values;
		for (int step = 0; step < 513; ++step) {
			int p = mode == 0 ? 0 : mode == 1 ? step : step / 2;
			T x = from_key<T, Bits>(U(rng()) & mask);
			wm.insert(p, x); values.insert(values.begin() + p, x);
			if (step % 31 == 0) snapshot(wm, values, rng);
			if (step % 19 == 0) { int q = int(rng() % values.size()); wm.set(q, x); values[q] = x; }
		}
		snapshot(wm, values, rng);
		for (int step = 0; step < 513; ++step) {
			int p = mode == 0 ? 0 : mode == 1 ? int(values.size()) - 1 : int(values.size()) / 2;
			assert(wm.erase(p) == values[p]); values.erase(values.begin() + p);
			if (step % 31 == 0) snapshot(wm, values, rng);
		}
		snapshot(wm, values, rng);
		wm.build({from_key<T, Bits>(0), from_key<T, Bits>(mask)});
		assert((wm.pop_back() == from_key<T, Bits>(mask)));
		assert((wm.pop_back() == from_key<T, Bits>(0)));
		assert(wm.empty());
	}
}

struct CustomBitVector : DynamicBitVector<128, 4> {
	inline static int calls = 0;
	std::pair<int, int> rank1(int l, int r) const { ++calls; return DynamicBitVector<128, 4>::rank1(l, r); }
};

void value_semantics() {
	using WM = DynamicWaveletMatrix<std::int64_t, 63>;
	static_assert(std::is_base_of_v<dynamic_wavelet_detail::QuadMatrix<std::int64_t, 63>, WM>);
	using Fallback = DynamicWaveletMatrix<std::int64_t, 63, CustomBitVector>;
	static_assert(!std::is_base_of_v<dynamic_wavelet_detail::QuadMatrix<std::int64_t, 63>, Fallback>);
	std::mt19937_64 rng(817);
	std::vector<std::int64_t> a{-7, 0, 2, 2, 8}, b{1, 1, 3};
	WM original(a), copied(original), assigned;
	assigned = original;
	snapshot(copied, a, rng); snapshot(assigned, a, rng);
	copied.set(0, 4); assert(original[0] == -7);
	WM moved(std::move(original)); assert(original.empty());
	original.push_back(7); assert(original.pop_back() == 7 && original.empty());
	snapshot(moved, a, rng);
	WM target(b); target = std::move(moved); assert(moved.empty());
	moved.build(b); snapshot(moved, b, rng);
	snapshot(target, a, rng);
	WM& alias = target; target = std::move(alias); snapshot(target, a, rng);
	target.swap(moved); snapshot(target, b, rng); snapshot(moved, a, rng);
	std::swap(target, moved); snapshot(target, a, rng); snapshot(moved, b, rng);
	target.clear(); assert(target.empty()); target.build(a); snapshot(target, a, rng);
	Fallback fallback(a); snapshot(fallback, a, rng); assert(CustomBitVector::calls > 0);
	fallback.set(1, -2); a[1] = -2; fallback.insert(2, 1); a.insert(a.begin() + 2, 1); assert(fallback.erase(3) == a[3]); a.erase(a.begin() + 3);
	snapshot(fallback, a, rng);
}

int main() {
	exhaustive<int, 1>(); exhaustive<unsigned, 1>(); exhaustive<int, 3>(); exhaustive<unsigned, 3>();
	odd_and_full<int, 7>(); odd_and_full<unsigned, 7>();
	odd_and_full<std::int32_t, 31>(); odd_and_full<std::uint32_t, 31>();
	odd_and_full<std::int64_t, 63>(); odd_and_full<std::uint64_t, 63>();
	odd_and_full<std::int64_t, 64>(); odd_and_full<std::uint64_t, 64>();
	structure<int, 7>(); structure<std::int64_t, 63>();
	value_semantics();
}
