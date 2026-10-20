#ifndef RADIX_SORT_HPP
#define RADIX_SORT_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

template <class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key key) {
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
	for (int shift = 0; shift < 64; shift += 8) {
		if (!((varying >> shift) & 255)) continue;
		std::array<std::size_t, 256> count{};
		for (const auto &value : values) ++count[(static_cast<std::uint64_t>(key(value)) >> shift) & 255];
		std::size_t sum = 0;
		for (auto &x : count) {
			auto n = x;
			x = sum, sum += n;
		}
		for (auto &&value : values) scratch[count[(static_cast<std::uint64_t>(key(value)) >> shift) & 255]++] = std::move(value);
		values.swap(scratch);
	}
}

#endif
