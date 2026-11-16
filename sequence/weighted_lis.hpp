#ifndef WEIGHTED_LIS_HPP
#define WEIGHTED_LIS_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <numeric>
#pragma push_macro("all")
#undef all
#include <ranges>
#pragma pop_macro("all")
#include <utility>
#include <vector>

namespace weighted_lis_detail {
template <class Range>
concept Sequence = std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>;

template <class Weight>
struct Result {
	Weight value{};
	int last = -1;
	std::vector<Weight> values;
	std::vector<int> parent;
};

template <bool Strict, bool Values, bool Parents, Sequence Range, Sequence Weights, class Compare>
auto scan(const Range &a, const Weights &weights, Compare compare) {
	assert(std::ranges::size(a) == std::ranges::size(weights));
	assert(std::ranges::size(a) <= std::size_t(std::numeric_limits<int>::max()));
	using Weight = std::ranges::range_value_t<const Weights>;
	Result<Weight> result;
	int n = int(std::ranges::size(a));
	if (!n) return result;
	if constexpr (Values) result.values.resize(n);
	if constexpr (Parents) result.parent.resize(n);
	auto first = std::ranges::begin(a);
	auto weight = std::ranges::begin(weights);
	std::vector<int> order(n), ranks(n);
	std::iota(order.begin(), order.end(), 0);
	std::sort(order.begin(), order.end(), [&](int i, int j) { return bool(compare(first[i], first[j])); });
	int count = 0;
	for (int i = 0; i < n; ++i) {
		if (i && compare(first[order[i - 1]], first[order[i]])) ++count;
		ranks[order[i]] = count;
	}
	struct Best { Weight value{}; int index = -1; };
	std::vector<Best> tree(std::size_t(count) + 2);
	for (int i = 0; i < n; ++i) {
		Best previous;
		for (unsigned r = unsigned(ranks[i]) + !Strict; r; r &= r - 1)
			if (previous.value < tree[r].value) previous = tree[r];
		Weight current = previous.value + weight[i];
		if constexpr (Values) result.values[i] = current;
		if constexpr (Parents) result.parent[i] = previous.index;
		if (result.value < current) result.value = current, result.last = i;
		if (Weight{} < current)
			for (std::size_t r = std::size_t(ranks[i]) + 1; r < tree.size(); r += r & -r)
				if (tree[r].value < current) tree[r] = {current, i};
	}
	return result;
}
}

template <bool Strict = true, weighted_lis_detail::Sequence Range, weighted_lis_detail::Sequence Weights, class Compare = std::less<>>
auto weighted_lis_value(const Range &a, const Weights &weights, Compare compare = {}) {
	return weighted_lis_detail::scan<Strict, false, false>(a, weights, std::move(compare)).value;
}

template <bool Strict = true, weighted_lis_detail::Sequence Range, weighted_lis_detail::Sequence Weights, class Compare = std::less<>>
auto weighted_lis_values(const Range &a, const Weights &weights, Compare compare = {}) {
	return weighted_lis_detail::scan<Strict, true, false>(a, weights, std::move(compare)).values;
}

template <bool Strict = true, weighted_lis_detail::Sequence Range, weighted_lis_detail::Sequence Weights, class Compare = std::less<>>
std::vector<int> weighted_lis(const Range &a, const Weights &weights, Compare compare = {}) {
	auto result = weighted_lis_detail::scan<Strict, false, true>(a, weights, std::move(compare));
	std::vector<int> indices;
	for (int i = result.last; i != -1; i = result.parent[i]) indices.push_back(i);
	std::reverse(indices.begin(), indices.end());
	return indices;
}

#endif
