#ifndef BINARY_SEQUENCE_HPP
#define BINARY_SEQUENCE_HPP

#include <algorithm>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string_view>
#include <type_traits>
#include <utility>

namespace binary_sequence_detail {

template <class Range>
concept RandomAccessRange = requires(const Range &values) {
	{ std::size(values) } -> std::convertible_to<std::size_t>;
	{ std::begin(values) } -> std::random_access_iterator;
	{ std::end(values) } -> std::sentinel_for<decltype(std::begin(values))>;
};

template <class Range>
using Iterator = decltype(std::begin(std::declval<const Range &>()));

template <class Range>
concept Sequence = RandomAccessRange<Range> &&
	std::is_integral_v<std::iter_value_t<Iterator<Range>>> && sizeof(std::iter_value_t<Iterator<Range>>) <= 8 &&
	(!std::is_volatile_v<std::remove_reference_t<std::iter_reference_t<Iterator<Range>>>>) &&
	(!std::convertible_to<const Range &, std::string_view>);

template <class Range>
concept Matrix = RandomAccessRange<Range> && Sequence<std::remove_cvref_t<std::iter_reference_t<Iterator<Range>>>>;

template <class Range> requires requires(const Range &values) { std::size(values); }
int size(const Range &values) {
	assert(std::size(values) <= std::size_t(INT_MAX));
	return int(std::size(values));
}

template <Sequence Range>
void pack(const Range &values, std::uint64_t *words) {
	int n = binary_sequence_detail::size(values);
	if (!n) return;
	std::fill_n(words, (std::size_t(n) + 63) / 64, std::uint64_t(0));
	auto first = std::begin(values);
	for (int i = 0; i < n; ++i) {
		auto value = first[i];
		assert(value == 0 || value == 1);
		if (bool(value)) words[unsigned(i) / 64] |= std::uint64_t(1) << (i & 63);
	}
}

}

#endif
