#ifndef WAVELET_COMPRESSION_HPP
#define WAVELET_COMPRESSION_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace wavelet_detail {
template<class T>
std::uint32_t lower_code(const std::vector<T> &values, bool dense, const T &x) {
	if (values.empty()) return 0;
	if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
		if (dense) {
			if (x <= values.front()) return 0;
			if (values.back() < x) return static_cast<int>(values.size());
			return std::uint64_t(x) - std::uint64_t(values.front());
		}
	int p = 0, length = static_cast<int>(values.size());
	while (length > 1) {
		int half = length / 2;
		p += (values[p + half] < x) * half;
		length -= half;
	}
	return p + (values[p] < x);
}

template<class T, bool WithPositions = true>
std::vector<std::uint32_t> compress(const std::vector<T> &a, std::vector<T> &values,
	int &ordered, std::vector<int> &positions, std::vector<int> &offsets) {
	assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
	const int n = static_cast<int>(a.size());
	ordered = 0;
	if constexpr (WithPositions) offsets.clear();
	std::vector<std::uint32_t> current(n);
	bool ascending = std::is_sorted(a.begin(), a.end());
	if (ascending || std::is_sorted(a.rbegin(), a.rend())) {
		ordered = ascending ? 1 : -1;
		values.clear();
		values.reserve(n);
		for (int i = 0; i < n; ++i) {
			int j = ascending ? i : n - 1 - i;
			if (values.empty() || values.back() != a[j]) values.push_back(a[j]);
			current[j] = static_cast<int>(values.size()) - 1;
		}
		if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
		if constexpr (WithPositions) positions.clear();
		return current;
	}
	if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8) {
		if (n >= 1024) {
			using U = std::make_unsigned_t<T>;
			struct [[gnu::packed]] Item {
				U key;
				std::uint32_t index;
			};
			std::vector<Item> order(n);
			U lo = std::numeric_limits<U>::max(), hi = 0;
			constexpr U sign = std::is_signed_v<T> ? U(1) << (sizeof(T) * 8 - 1) : U(0);
			for (int i = 0; i < n; ++i) {
				U key = U(a[i]) ^ sign;
				order[i] = {key, std::uint32_t(i)};
				lo = std::min(lo, key);
				hi = std::max(hi, key);
			}
			if (hi - lo < 65536 && std::uint64_t(hi - lo) < unsigned(n)) {
				std::vector<int> codes(std::size_t(hi - lo) + 1, -1);
				for (const auto &v : order) codes[v.key - lo] = 0;
				values.clear();
				values.reserve(codes.size());
				for (std::size_t i = 0; i < codes.size(); ++i)
					if (codes[i] == 0) {
						codes[i] = static_cast<int>(values.size());
						values.push_back(T(U(lo + U(i)) ^ sign));
					}
				for (const auto &v : order) current[v.index] = codes[v.key - lo];
				if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
				if constexpr (WithPositions) positions.clear();
				return current;
			}
			std::vector<Item> next(n);
			int width = std::bit_width(U(lo ^ hi)), span_width = std::bit_width(U(hi - lo));
			if ((width + 10) / 11 > (span_width + 10) / 11) {
				for (auto &v : order) v.key -= lo;
				width = span_width;
			}
			for (int shift = 0; shift < width; shift += 11) {
				std::array<int, 2048> offsets{};
				for (const auto &v : order) ++offsets[(v.key >> shift) & 2047];
				int total = 0;
				for (int &c : offsets) {
					int t = c;
					c = total;
					total += t;
				}
				for (const auto &v : order) next[offsets[(v.key >> shift) & 2047]++] = v;
				order.swap(next);
			}
			values.clear();
			values.reserve(n);
			U previous = 0;
			if constexpr (WithPositions) {
				positions.resize(n);
				offsets.reserve(std::size_t(n) + 1);
			}
			for (int j = 0; j < n; ++j) {
				const auto &v = order[j];
				if (values.empty() || v.key != previous) {
					values.push_back(a[v.index]);
					if constexpr (WithPositions) offsets.push_back(j);
				}
				previous = v.key;
				current[v.index] = static_cast<int>(values.size()) - 1;
				if constexpr (WithPositions) positions[j] = v.index;
			}
			if constexpr (WithPositions) offsets.push_back(n);
			if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
			return current;
		}
	}
	values = a;
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
	for (int i = 0; i < n; ++i) current[i] = lower_code(values, false, a[i]);
	if constexpr (WithPositions) positions.clear();
	return current;
}

template<class T> struct ValueIndex {
	int n = 0;
	std::vector<T> values;
	std::vector<std::uint32_t> original;
	void build(const std::vector<T> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		n = static_cast<int>(a.size());
		int ordered = 0;
		std::vector<int> positions, offsets;
		original = compress<T, false>(a, values, ordered, positions, offsets);
	}
	std::uint32_t code(const T &x) const { return lower_code(values, false, x); }
};
}

#endif
