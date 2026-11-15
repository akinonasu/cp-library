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
#include <span>
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

template <class Sequence>
concept IntegerSequence = std::ranges::contiguous_range<const Sequence> && std::ranges::sized_range<const Sequence> &&
	std::is_integral_v<std::ranges::range_value_t<const Sequence>> &&
	!std::is_same_v<std::ranges::range_value_t<const Sequence>, bool> && sizeof(std::ranges::range_value_t<const Sequence>) <= 8 &&
	!std::is_volatile_v<std::remove_reference_t<std::ranges::range_reference_t<const Sequence>>>;

template <class Range>
concept IntegerSequences = !Strings<Range> && std::ranges::forward_range<const Range> &&
	std::is_lvalue_reference_v<std::ranges::range_reference_t<const Range>> &&
	IntegerSequence<std::remove_reference_t<std::ranges::range_reference_t<const Range>>>;

template <class Range>
using Symbol = std::ranges::range_value_t<std::ranges::range_value_t<const Range>>;

struct Entry {
	std::string_view text;
	std::vector<int> z;
	std::uint64_t compared = 0;
};

template <class T>
struct IntegerEntry {
	std::span<const T> text;
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

template <IntegerSequences Range>
std::vector<IntegerEntry<Symbol<Range>>> prepare(const Range &sequences) {
	std::vector<IntegerEntry<Symbol<Range>>> entries;
	[[maybe_unused]] constexpr auto maximum = std::numeric_limits<int>::max();
	if constexpr (std::ranges::sized_range<const Range>) {
		assert(std::ranges::size(sequences) <= std::size_t(maximum));
		entries.reserve(std::ranges::size(sequences));
	}
	for (const auto &sequence : sequences) {
		assert(entries.size() < std::size_t(maximum) && std::ranges::size(sequence) <= std::size_t(maximum));
		entries.push_back({std::span<const Symbol<Range>>(std::ranges::data(sequence), std::ranges::size(sequence)), {}});
	}
	return entries;
}

template <class T>
inline int lex_compare(const T *a, const T *b, int size) {
	int common = sequence_compare_detail::prefix(a, b, size);
	return common == size ? 0 : (a[common] > b[common]) - (a[common] < b[common]);
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

template <class T>
inline int compare(IntegerEntry<T> &first, IntegerEntry<T> &second) {
	IntegerEntry<T> *a = &first, *b = &second;
	bool reverse = a->text.size() > b->text.size();
	if (reverse) std::swap(a, b);
	int n = int(a->text.size()), m = int(b->text.size());
	int value = lex_compare(a->text.data(), b->text.data(), n);
	if (!value && n < m) {
		if (m - n <= n) value = lex_compare(b->text.data(), b->text.data() + n, m - n);
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
			if (common < m - n) value = (b->text[common] > b->text[n + common]) - (b->text[common] < b->text[n + common]);
		}
		if (!value) value = lex_compare(b->text.data() + m - n, a->text.data(), n);
	}
	return reverse ? -value : value;
}

template <class EntryType>
inline std::vector<int> order(std::vector<EntryType> &entries, bool descending) {
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

template <IntegerSequences Range>
std::vector<Symbol<Range>> concatenate(const Range &sequences, bool descending) {
	auto entries = prepare(sequences);
	auto indices = order(entries, descending);
	std::vector<Symbol<Range>> result;
	std::size_t length = 0;
	for (const auto &entry : entries) {
		if (entry.text.size() > result.max_size() - length) throw std::length_error("concatenation");
		length += entry.text.size();
	}
	result.reserve(length);
	for (int i : indices) if (!entries[i].text.empty()) result.insert(result.end(), entries[i].text.begin(), entries[i].text.end());
	return result;
}
}

template <class S, std::size_t SizeS, class T, std::size_t SizeT>
requires (std::is_integral_v<S> && !std::is_same_v<std::remove_const_t<S>, bool> && sizeof(S) <= 8 && std::is_same_v<std::remove_const_t<S>, std::remove_const_t<T>> && !std::is_volatile_v<S> && !std::is_volatile_v<T>)
int concatenation_compare(std::span<S, SizeS> first, std::span<T, SizeT> second) {
	using Value = std::remove_const_t<S>;
	std::span<const Value> a = first, b = second;
	assert(a.size() <= std::size_t(std::numeric_limits<int>::max()) && b.size() <= std::size_t(std::numeric_limits<int>::max()));
	if (a.empty() || b.empty()) return 0;
	bool reverse = a.size() > b.size();
	if (reverse) std::swap(a, b);
	int n = int(a.size()), m = int(b.size());
	int value = concatenation_order_detail::lex_compare(a.data(), b.data(), n);
	if (!value && n < m) {
		value = concatenation_order_detail::lex_compare(b.data(), b.data() + n, m - n);
		if (!value) value = concatenation_order_detail::lex_compare(b.data() + m - n, a.data(), n);
	}
	return reverse ? -value : value;
}

template <class T, class AllocatorA, class AllocatorB>
requires (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8)
int concatenation_compare(const std::vector<T, AllocatorA> &a, const std::vector<T, AllocatorB> &b) {
	return concatenation_compare(std::span<const T>(a.data(), a.size()), std::span<const T>(b.data(), b.size()));
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

template <concatenation_order_detail::IntegerSequences Range>
std::vector<int> concatenation_order(const Range &sequences, bool descending = false) {
	auto entries = concatenation_order_detail::prepare(sequences);
	return concatenation_order_detail::order(entries, descending);
}

template <concatenation_order_detail::IntegerSequences Range>
std::vector<concatenation_order_detail::Symbol<Range>> min_concatenation(const Range &sequences) {
	return concatenation_order_detail::concatenate(sequences, false);
}

template <concatenation_order_detail::IntegerSequences Range>
std::vector<concatenation_order_detail::Symbol<Range>> max_concatenation(const Range &sequences) {
	return concatenation_order_detail::concatenate(sequences, true);
}

#endif
