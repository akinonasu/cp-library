#ifndef OFFLINE_RANGE_SUM_HPP
#define OFFLINE_RANGE_SUM_HPP

#include "../components/insert_only_prefix_count_sum.hpp"
#include "../components/radix_sort.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class T, class Sum = std::conditional_t<std::is_integral_v<T>, std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>, T>>
class OfflineRangeSum {
	struct Query { T upper; int first, last, index; };
	struct Item { T value; int position; };
	std::vector<T> values;
	std::vector<Query> queries;
  public:
	using sum_type = Sum;
	OfflineRangeSum() = default;
	explicit OfflineRangeSum(const std::vector<T> &a) { build(a); }
	void build(const std::vector<T> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		values = a; queries.clear();
	}
	int size() const { return int(values.size()); }
	void reserve(int count) { assert(count >= 0); queries.reserve(count); }
	int add_query(int l, int r, const T &upper) {
		assert(0 <= l && l <= r && r <= size() && queries.size() < std::size_t(std::numeric_limits<int>::max()));
		queries.push_back({upper, l, r, int(queries.size())});
		return int(queries.size()) - 1;
	}
	std::vector<std::pair<Sum, int>> solve() const {
		std::vector<std::pair<Sum, int>> result(queries.size());
		if (queries.empty() || values.empty()) return result;
		std::vector<Item> order;
		order.reserve(values.size());
		for (int i = 0; i < size(); ++i) order.push_back({values[i], i});
		radix_sort_detail::sort_by_key<11>(order, [](const Item &item) { return item.value; });
		auto sorted = queries;
		radix_sort_detail::sort_by_key<11>(sorted, [](const Query &query) { return query.upper; });
		if constexpr (std::is_integral_v<T> && sizeof(T) <= 8 && (std::is_integral_v<Sum> || std::is_same_v<Sum, __int128_t> || std::is_same_v<Sum, __uint128_t>)) {
			int block_length = std::min(size(), insert_only_prefix_detail::block_size);
			__int128_t base = order.front().value;
			__uint128_t bound = (__int128_t(order.back().value) - base) * __uint128_t(block_length);
			int bits = 1;
			while (bound >>= 1) ++bits;
			if (bits + std::bit_width(unsigned(block_length)) <= 64) {
				insert_only_prefix_detail::Packed tree(size(), base, bits);
				std::size_t inserted = 0;
				for (const auto &query : sorted) {
					while (inserted < order.size() && order[inserted].value < query.upper) {
						const auto &item = order[inserted++];
						tree.insert(item.position, std::uint64_t(__int128_t(item.value) - base));
					}
					auto answer = tree.fold(query.first, query.last);
					result[query.index] = {Sum(answer.sum), answer.count};
				}
				return result;
			}
		}
		using Accum = std::conditional_t<std::is_integral_v<Sum> && sizeof(Sum) <= 8, __int128_t, Sum>;
		struct Payload {
			Accum sum{}; int count = 0;
			Payload &operator+=(const Payload &other) { sum += other.sum; count += other.count; return *this; }
			Payload &operator-=(const Payload &other) { sum -= other.sum; count -= other.count; return *this; }
		};
		FenwickSum<Payload> tree(size());
		std::size_t inserted = 0;
		for (const auto &query : sorted) {
			while (inserted < order.size() && order[inserted].value < query.upper) {
				const auto &item = order[inserted++];
				tree.add(item.position, {Accum(item.value), 1});
			}
			auto answer = tree.prod(query.first, query.last);
			result[query.index] = {Sum(answer.sum), answer.count};
		}
		return result;
	}
};

#endif
