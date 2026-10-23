#ifndef STATIC_RANGE_INVERSIONS_HPP
#define STATIC_RANGE_INVERSIONS_HPP

#include "inversion_count.hpp"
#include "../components/radix_sort.hpp"
#include "../data_structure/range_query/mo_order.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

template<class Range, class Compare = std::less<>>
	requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>
std::vector<std::int64_t> static_range_inversion_counts(const Range &values,
	const std::vector<std::pair<int, int>> &ranges, Compare compare = {}) {
	assert(std::ranges::size(values) <= std::size_t(std::numeric_limits<int>::max()));
	assert(ranges.size() <= std::size_t(std::numeric_limits<int>::max()));
	const int n = int(std::ranges::size(values)), q = int(ranges.size());
	for ([[maybe_unused]] auto [l, r] : ranges) assert(0 <= l && l <= r && r <= n);
	std::vector<std::int64_t> answer(q);
	if (!q || n < 2) return answer;
	auto first = std::ranges::begin(values);
	if (q == 1) {
		auto [l, r] = ranges[0];
		answer[0] = inversion_count(std::ranges::subrange(first + l, first + r), compare);
		return answer;
	}
	using T = std::ranges::range_value_t<const Range>;
	std::vector<int> ordered(n), rank(n);
	std::iota(ordered.begin(), ordered.end(), 0);
	if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8 &&
		(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>)) {
		using U = std::make_unsigned_t<T>;
		stable_radix_sort(ordered, [&](int i) {
			U key = U(first[i]);
			if constexpr (std::is_signed_v<T>) key ^= U(1) << (std::numeric_limits<U>::digits - 1);
			if constexpr (std::is_same_v<Compare, std::greater<>>) key = U(~key);
			return key;
		});
	} else {
		std::sort(ordered.begin(), ordered.end(), [&](int a, int b) {
			if (compare(first[a], first[b])) return true;
			return !compare(first[b], first[a]) && a < b;
		});
	}
	for (int i = 0; i < n; ++i) rank[ordered[i]] = i;
	auto queries = mo_order(ranges);
	struct Event {
		std::size_t next;
		int left, right, query, kind;
	};
	constexpr std::size_t none = std::numeric_limits<std::size_t>::max();
	std::vector<std::size_t> heads(n, none);
	std::vector<Event> events;
	events.reserve(std::size_t(q) * 2);
	auto add = [&](int sweep, int l, int r, int sign, bool length, int query) {
		if (sweep >= 0) {
			events.push_back({heads[sweep], l, r, query, sign * (1 + int(length))});
			heads[sweep] = events.size() - 1;
		}
	};
	int left = 0, right = 0;
	for (int i : queries) {
		auto [l, r] = ranges[i];
		if (right < r) add(left - 1, right, r, 1, false, i), right = r;
		if (l < left) add(right - 1, l, left, 1, true, i), left = l;
		if (r < right) add(left - 1, r, right, -1, false, i), right = r;
		if (left < l) add(right - 1, left, l, -1, true, i), left = l;
	}
	int shift = 0;
	while ((std::uint64_t(1) << (2 * shift + 2)) <= std::uint64_t(n)) ++shift;
	const int block = 1 << shift;
	std::vector<int> direct(n), lazy((std::size_t(n) + block - 1) >> shift);
	std::vector<std::int64_t> prefix(std::size_t(n) + 1), delta(q);
	for (int sweep = 0; sweep < n; ++sweep) {
		const int key = rank[sweep], group = key >> shift;
		prefix[sweep + 1] = prefix[sweep] + direct[key] + lazy[group];
		for (int i = 0; i < group; ++i) ++lazy[i];
		for (int i = group << shift; i <= key; ++i) ++direct[i];
		for (std::size_t i = heads[sweep]; i != none; i = events[i].next) {
			const auto &event = events[i];
			std::int64_t sum = 0;
			for (int p = event.left; p < event.right; ++p) {
				int key = rank[p];
				sum += int(event.kind == 2 || event.kind == -2) * (sweep - p + 1) - direct[key] - lazy[key >> shift];
			}
			delta[event.query] += event.kind > 0 ? sum : -sum;
		}
	}
	std::int64_t sum = 0;
	for (int i : queries) {
		sum += delta[i];
		auto [l, r] = ranges[i];
		answer[i] = sum + prefix[r] - prefix[l];
	}
	return answer;
}

#endif
