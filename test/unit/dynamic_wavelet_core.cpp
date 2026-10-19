#include "components/dynamic_bit_vector.hpp"
#include "data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

struct RankSelectBits : DynamicBitVector<128, 4> {
	void access_rank(int) const = delete;
};

template <class T, int Bits>
T decode(std::make_unsigned_t<T> key) {
	using U = std::make_unsigned_t<T>;
	constexpr int width = std::numeric_limits<U>::digits;
	constexpr U mask = std::numeric_limits<U>::max() >> (width - Bits);
	constexpr U sign = std::is_signed_v<T> ? U(1) << (Bits - 1) : 0;
	key ^= sign;
	if constexpr (std::is_signed_v<T> && Bits < width)
		if (key & sign) key |= U(~mask);
	return T(key);
}

template <class WM, class T>
void check_select(const WM &wm, const std::vector<T> &a, T value) {
	int k = 0;
	assert(wm.select(value, -1) == -1);
	for (int i = 0; i < int(a.size()); ++i)
		if (a[i] == value) assert(wm.select(value, k++) == i);
	assert(wm.select(value, k) == -1);
	assert(wm.select(value, int(a.size())) == -1);
}

template <class WM, class T>
void check_range(const WM &wm, const std::vector<T> &a, int l, int r, T low, T high) {
	int count = std::count_if(a.begin() + l, a.begin() + r, [&](T value) { return low <= value && value < high; });
	assert(wm.range_freq(l, r, low, high) == count);
}

void word_select() {
	DynamicBitVector<128, 4> mixed;
	mixed.build(65, std::vector<std::uint64_t>{~std::uint64_t(0), 0});
	assert(mixed.select1(63) == 63 && mixed.select0(0) == 64 && mixed.select1(64) == -1);
	mixed.build(65, std::vector<std::uint64_t>{0, 1});
	assert(mixed.select0(63) == 63 && mixed.select1(0) == 64 && mixed.select0(64) == -1);
	std::mt19937_64 random(21371);
	for (int n : {1, 2, 7, 8, 15, 31, 32, 63, 64, 65, 127, 128, 129}) {
		for (int rep = 0; rep < 80; ++rep) {
			std::vector<std::uint64_t> packed((n + 63) / 64);
			for (auto &word : packed) word = rep < 64 ? std::uint64_t(1) << rep : rep == 64 ? ~std::uint64_t(0) : rep == 65 ? 0 : random();
			DynamicBitVector<128, 4> bits;
			bits.build(n, packed);
			for (bool value : {false, true}) {
				int k = 0;
				for (int i = 0; i < n; ++i)
					if (bool((packed[i >> 6] >> (i & 63)) & 1) == value) assert(bits.select(value, k++) == i);
				assert(bits.select(value, -1) == -1 && bits.select(value, k) == -1);
			}
		}
	}
}

template <class T, int Bits>
void pack_edges() {
	using U = std::make_unsigned_t<T>;
	constexpr U mask = std::numeric_limits<U>::max() >> (std::numeric_limits<U>::digits - Bits);
	DynamicWaveletMatrix<T, Bits> wm;
	for (int n : {0, 1, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 65, 127, 128, 129}) {
		for (int shift = 0; shift < Bits; ++shift) {
			std::vector<T> a(n);
			for (int i = 0; i < n; ++i) a[i] = decode<T, Bits>(i % 3 == 0 ? mask : i % 3 == 1 ? U(1) << shift : U(0));
			wm.build(a);
			for (int i = 0; i < n; ++i) assert(wm.access(i) == a[i]);
			for (T value : {decode<T, Bits>(0), decode<T, Bits>(mask), decode<T, Bits>(U(1) << shift)})
				check_select(wm, a, value);
		}
	}
}

template <class T, int Bits, class BV = DynamicBitVector<>>
void range_boundaries() {
	using U = std::make_unsigned_t<T>;
	constexpr U mask = std::numeric_limits<U>::max() >> (std::numeric_limits<U>::digits - Bits);
	std::vector<T> a;
	for (int i = 0; i < Bits; ++i) {
		a.push_back(decode<T, Bits>(U(1) << i));
		a.push_back(decode<T, Bits>(mask ^ (U(1) << i)));
	}
	a.push_back(decode<T, Bits>(0));
	a.push_back(decode<T, Bits>(mask));
	DynamicWaveletMatrix<T, Bits, BV> wm(a);
	std::vector<T> bounds{std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), T(0), T(1)};
	bounds.insert(bounds.end(), a.begin(), a.end());
	std::sort(bounds.begin(), bounds.end());
	bounds.erase(std::unique(bounds.begin(), bounds.end()), bounds.end());
	int n = int(a.size());
	for (int pass = 0; pass < 4; ++pass) {
		for (T low : bounds) {
			check_select(wm, a, low);
			for (T high : bounds)
				for (auto [l, r] : {std::pair(0, n), std::pair(1, n - 1), std::pair(0, 0), std::pair(n / 2, n / 2 + 1)})
					check_range(wm, a, l, r, low, high);
		}
		if (pass == 0) wm.set(0, a.back()), a[0] = a.back();
		if (pass == 1) wm.insert(1, a.back()), a.insert(a.begin() + 1, a.back()), ++n;
		if (pass == 2) {
			assert(wm.erase(1) == a[1]);
			a.erase(a.begin() + 1);
			--n;
		}
	}
}

template <class BV>
void select_suffix() {
	using T = std::uint64_t;
	std::vector<T> a(65);
	for (int i = 0; i < int(a.size()); ++i) a[i] = T(i) << 32;
	DynamicWaveletMatrix<T, 64, BV> wm(a);
	for (int i = 0; i < int(a.size()); ++i) {
		check_select(wm, a, a[i]);
		for (int bit = 0; bit < 64; ++bit) check_select(wm, a, T(a[i] ^ (T(1) << bit)));
	}
	T absent = (T(1) << 63) | 1;
	check_select(wm, a, absent);
	T old = a[7];
	wm.set(7, absent), a[7] = absent;
	check_select(wm, a, absent);
	wm.insert(0, absent), a.insert(a.begin(), absent);
	check_select(wm, a, absent);
	assert(wm.erase(0) == absent);
	a.erase(a.begin());
	check_select(wm, a, absent);
	wm.set(7, old), a[7] = old;
	check_select(wm, a, absent);
	wm.build(std::vector<T>{absent});
	check_select(wm, std::vector<T>{absent}, absent);
	check_select(wm, std::vector<T>{absent}, absent ^ 1);
	wm.clear();
	check_select(wm, std::vector<T>{}, absent);
}

void small_ranges() {
	for (int n = 0, limit = 1; n <= 5; ++n, limit *= 3) {
		for (int pattern = 0; pattern < limit; ++pattern) {
			std::vector<int> a(n);
			for (int i = 0, code = pattern; i < n; ++i, code /= 3) a[i] = code % 3 - 1;
			DynamicWaveletMatrix<int, 3> wm(a);
			for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r)
				for (int low = -5; low <= 4; ++low) for (int high = -5; high <= 4; ++high)
					check_range(wm, a, l, r, low, high);
		}
	}
}

int main() {
	word_select();
	pack_edges<std::uint32_t, 1>();
	pack_edges<std::uint32_t, 31>();
	pack_edges<std::uint32_t, 32>();
	pack_edges<std::int32_t, 1>();
	pack_edges<std::int32_t, 31>();
	pack_edges<std::int32_t, 32>();
	pack_edges<std::uint64_t, 33>();
	pack_edges<std::uint64_t, 63>();
	pack_edges<std::uint64_t, 64>();
	pack_edges<std::int64_t, 33>();
	pack_edges<std::int64_t, 63>();
	pack_edges<std::int64_t, 64>();
	pack_edges<std::uint8_t, 8>();
	pack_edges<std::int8_t, 8>();
	pack_edges<std::uint16_t, 16>();
	pack_edges<std::int16_t, 16>();
	select_suffix<DynamicBitVector<>>();
	select_suffix<RankSelectBits>();
	small_ranges();
	range_boundaries<int, 1>();
	range_boundaries<unsigned, 1>();
	range_boundaries<int, 12>();
	range_boundaries<unsigned, 20>();
	range_boundaries<std::uint8_t, 8>();
	range_boundaries<std::int8_t, 8>();
	range_boundaries<std::uint16_t, 16>();
	range_boundaries<std::int16_t, 16>();
	range_boundaries<std::int64_t, 33>();
	range_boundaries<std::uint64_t, 63>();
	range_boundaries<std::int64_t, 64>();
	range_boundaries<std::uint64_t, 64>();
	range_boundaries<int, 12, RankSelectBits>();
}
