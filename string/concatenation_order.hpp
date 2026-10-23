#ifndef CONCATENATION_ORDER_HPP
#define CONCATENATION_ORDER_HPP

#include "../components/sequence_compare.hpp"
#include "../components/z_algorithm.hpp"
#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

inline int concatenation_compare(std::string_view a, std::string_view b) {
	if (a.empty() || b.empty()) return 0;
	bool reverse = a.size() > b.size();
	if (reverse) std::swap(a, b);
	std::size_t n = a.size(), m = b.size();
	int value = std::char_traits<char>::compare(a.data(), b.data(), n);
	if (!value && n < m) {
		value = std::char_traits<char>::compare(b.data(), b.data() + n, m - n);
		if (!value) value = std::char_traits<char>::compare(b.data() + m - n, a.data(), n);
	}
	int result = (value > 0) - (value < 0);
	return reverse ? -result : result;
}

namespace concatenation_order_detail {
template <class Range>
concept Strings = std::ranges::forward_range<const Range> &&
	std::is_lvalue_reference_v<std::ranges::range_reference_t<const Range>> &&
	std::convertible_to<std::ranges::range_reference_t<const Range>, std::string_view>;

struct Entry {
	std::string_view text;
	std::vector<int> z;
	std::uint64_t compared = 0;
};

template <Strings Range>
std::vector<Entry> prepare(const Range &strings) {
	std::vector<Entry> entries;
	[[maybe_unused]] constexpr auto maximum = std::numeric_limits<int>::max();
	if constexpr (std::ranges::sized_range<const Range>) {
		assert(std::ranges::size(strings) <= std::size_t(maximum));
		entries.reserve(std::ranges::size(strings));
	}
	for (const auto &s : strings) {
		std::string_view text = s;
		assert(entries.size() < std::size_t(maximum) && text.size() <= std::size_t(maximum));
		entries.push_back({text, {}});
	}
	return entries;
}

inline int compare(Entry &first, Entry &second) {
	Entry *a = &first, *b = &second;
	bool reverse = a->text.size() > b->text.size();
	if (reverse) std::swap(a, b);
	int n = int(a->text.size()), m = int(b->text.size());
	int value = std::char_traits<char>::compare(a->text.data(), b->text.data(), n);
	if (!value && n < m) {
		if (m - n <= n) value = std::char_traits<char>::compare(b->text.data(), b->text.data() + n, m - n);
		else {
			int common;
			if (b->z.empty() && b->compared < 8 * std::uint64_t(m)) {
				common = sequence_compare_detail::prefix(b->text.data(), b->text.data() + n, m - n);
				b->compared += common + 1;
			} else {
				if (b->z.empty()) {
					b->z.resize(m);
					auto s = b->text;
					sequence_match_detail::z_table(m, b->z.data(), [&](int i, int j, int limit) {
						return sequence_compare_detail::prefix(s.data() + i, s.data() + j, limit);
					});
				}
				common = b->z[n];
			}
			if (common < m - n) value = int(static_cast<unsigned char>(b->text[common])) - int(static_cast<unsigned char>(b->text[n + common]));
		}
		if (!value) value = std::char_traits<char>::compare(b->text.data() + m - n, a->text.data(), n);
	}
	int result = (value > 0) - (value < 0);
	return reverse ? -result : result;
}

inline std::vector<int> order(std::vector<Entry> &entries, bool descending) {
	std::vector<int> result;
	result.reserve(entries.size());
	for (int i = 0; i < int(entries.size()); ++i) if (entries[i].text.empty()) result.push_back(i);
	std::size_t empty = result.size();
	for (int i = 0; i < int(entries.size()); ++i) if (!entries[i].text.empty()) result.push_back(i);
	std::size_t n = result.size() - empty;
	if (n <= 1) return result;
	std::vector<int> buffer(n);
	int *source = result.data() + empty, *target = buffer.data();
	for (std::size_t width = 1; width < n; width *= 2) {
		for (std::size_t left = 0; left < n; left += 2 * width) {
			std::size_t middle = std::min(left + width, n), right = std::min(left + 2 * width, n);
			if (middle < right) {
				int value = compare(entries[source[middle - 1]], entries[source[middle]]);
				if (descending ? value >= 0 : value <= 0) {
					std::copy(source + left, source + right, target + left);
					continue;
				}
			}
			std::size_t i = left, j = middle, k = left;
			while (i < middle && j < right) {
				int value = compare(entries[source[i]], entries[source[j]]);
				bool take_right = descending ? value < 0 : value > 0;
				target[k++] = take_right ? source[j++] : source[i++];
			}
			while (i < middle) target[k++] = source[i++];
			while (j < right) target[k++] = source[j++];
		}
		std::swap(source, target);
	}
	if (source != result.data() + empty) std::copy_n(source, n, result.data() + empty);
	return result;
}

template <Strings Range>
std::string concatenate(const Range &strings, bool descending) {
	auto entries = prepare(strings);
	auto indices = order(entries, descending);
	std::string result;
	std::size_t length = 0;
	for (const auto &entry : entries) {
		if (entry.text.size() > result.max_size() - length) throw std::length_error("concatenation");
		length += entry.text.size();
	}
	result.reserve(length);
	for (int i : indices) if (!entries[i].text.empty()) result.append(entries[i].text);
	return result;
}
}

template <concatenation_order_detail::Strings Range>
std::vector<int> concatenation_order(const Range &strings, bool descending = false) {
	auto entries = concatenation_order_detail::prepare(strings);
	return concatenation_order_detail::order(entries, descending);
}

template <concatenation_order_detail::Strings Range>
std::string min_concatenation(const Range &strings) {
	return concatenation_order_detail::concatenate(strings, false);
}

template <concatenation_order_detail::Strings Range>
std::string max_concatenation(const Range &strings) {
	return concatenation_order_detail::concatenate(strings, true);
}

#endif
