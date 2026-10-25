#ifndef RADIX_SORT_HPP
#define RADIX_SORT_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

template <int Bits, class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key &&key) {
	static_assert(1 <= Bits && Bits <= 16);
	constexpr unsigned buckets = 1U << Bits, mask = buckets - 1;
	using K = std::remove_cvref_t<decltype(key(std::declval<const T &>()))>;
	static_assert(std::is_integral_v<K> && std::is_unsigned_v<K> && sizeof(K) <= sizeof(std::uint64_t));
	if (values.size() < 2) return;
	if (values.size() < 256) {
		std::stable_sort(values.begin(), values.end(), [&](const T &a, const T &b) { return key(a) < key(b); });
		return;
	}
	std::uint64_t first = key(values.front()), previous = first, varying = 0;
	bool sorted = true;
	for (std::size_t i = 1; i < values.size(); ++i) {
		std::uint64_t current = key(values[i]);
		varying |= first ^ current;
		sorted &= previous <= current;
		previous = current;
	}
	if (sorted) return;
	std::vector<T> scratch(values.size());
	using Counts = std::conditional_t<(Bits <= 11), std::array<std::size_t, buckets>, std::vector<std::size_t>>;
	Counts count = [] {
		if constexpr (Bits <= 11) return Counts{};
		else return Counts(buckets);
	}();
	for (int shift = 0; shift < 64; shift += Bits) {
		if (!((varying >> shift) & mask)) continue;
		std::fill(count.begin(), count.end(), 0);
		for (const auto &value : values) ++count[(static_cast<std::uint64_t>(key(value)) >> shift) & mask];
		std::size_t sum = 0;
		for (auto &x : count) {
			auto n = x;
			x = sum, sum += n;
		}
		for (auto &&value : values) scratch[count[(static_cast<std::uint64_t>(key(value)) >> shift) & mask]++] = std::move(value);
		values.swap(scratch);
	}
}

template <class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key key) {
	stable_radix_sort<8>(values, std::move(key));
}

namespace radix_sort_detail {
template <int Bits = 8, class T, class Key>
void sort_by_key(std::vector<T> &values, Key key) {
	using K = std::remove_cvref_t<decltype(key(std::declval<const T &>()))>;
	if constexpr (std::is_integral_v<K> && !std::is_same_v<K, bool> && sizeof(K) <= 8 && std::is_default_constructible_v<T>) {
		using U = std::make_unsigned_t<K>;
		constexpr U sign = std::is_signed_v<K> ? U(1) << (sizeof(K) * 8 - 1) : U(0);
		stable_radix_sort<Bits>(values, [&](const T &value) { return U(U(key(value)) ^ sign); });
	} else std::sort(values.begin(), values.end(), [&](const T &a, const T &b) { return key(a) < key(b); });
}
}

#endif
