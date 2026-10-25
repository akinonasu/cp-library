#ifndef OFFLINE_RANGE_COUNT_DISTINCT_HPP
#define OFFLINE_RANGE_COUNT_DISTINCT_HPP

#include "flat_hash_map.hpp"
#include "../components/incremental_prefix_count.hpp"
#include <array>
#include <cassert>
#include <functional>
#include <limits>
#include <vector>

template <class T, class Hash = std::hash<T>, class Equal = std::equal_to<T>>
class OfflineRangeCountDistinct {
	std::vector<T> values;
	std::vector<std::array<int, 2>> queries;
  public:
	OfflineRangeCountDistinct() = default;
	explicit OfflineRangeCountDistinct(const std::vector<T> &a) { build(a); }
	void build(const std::vector<T> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		values = a; queries.clear();
	}
	int size() const { return int(values.size()); }
	void reserve(int count) { assert(count >= 0); queries.reserve(count); }
	int add_query(int l, int r) {
		assert(0 <= l && l <= r && r <= size() && queries.size() < std::size_t(std::numeric_limits<int>::max()));
		queries.push_back({l, r});
		return int(queries.size()) - 1;
	}
	std::vector<int> solve() const {
		std::vector<int> result(queries.size());
		if (queries.empty()) return result;
		std::vector<int> ends(std::size_t(size()) + 2), order(queries.size());
		for (auto query : queries) ++ends[std::size_t(query[1]) + 1];
		for (std::size_t i = 1; i < ends.size(); ++i) ends[i] += ends[i - 1];
		auto cursor = ends;
		for (int i = 0; i < int(queries.size()); ++i) order[cursor[queries[i][1]]++] = i;
		FlatHashMap<T, int, Hash, Equal> last;
		last.reserve(values.size());
		IncrementalPrefixCount positions(size());
		for (std::size_t r = 0; r <= values.size(); ++r) {
			if (r) {
				int &previous = last[values[r - 1]];
				positions.increment(previous);
				previous = r;
			}
			for (int i = ends[r]; i < ends[r + 1]; ++i) {
				int id = order[i];
				int l = queries[id][0];
				result[id] = std::size_t(l) == r ? 0 : positions.prefix(l + 1) - l;
			}
		}
		return result;
	}
};

#endif
