#ifndef RUN_LENGTH_ENCODING_HPP
#define RUN_LENGTH_ENCODING_HPP

#include <cassert>
#include <cstdint>
#include <limits>
#include <functional>
#include <ranges>
#include <utility>
#include <vector>

template<class Range, class Equal = std::equal_to<>>
	requires std::ranges::input_range<const Range>
auto run_length_encode(const Range &a, Equal equal = {}) {
	using T = std::ranges::range_value_t<const Range>;
	std::vector<std::pair<T, std::int64_t>> result;
	auto it = std::ranges::begin(a);
	const auto last = std::ranges::end(a);
	while (it != last) {
		T value = *it;
		std::int64_t count = 1;
		for (++it; it != last && equal(value, *it); ++it) {
			assert(count < std::numeric_limits<std::int64_t>::max());
			++count;
		}
		result.emplace_back(std::move(value), count);
	}
	return result;
}

#endif
