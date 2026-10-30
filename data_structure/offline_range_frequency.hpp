#ifndef OFFLINE_RANGE_FREQUENCY_HPP
#define OFFLINE_RANGE_FREQUENCY_HPP

#include "flat_hash_map.hpp"
#include "../components/fenwick_bitset.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <limits>
#include <vector>

template <class T, class Hash = std::hash<T>, class Equal = std::equal_to<T>>
class OfflineRangeFrequency {
	struct Event { int value, first, last, answer, delta; };
	FlatHashMap<T, int, Hash, Equal> ids;
	std::vector<int> initial, current;
	std::vector<Event> events;
	int answers = 0;
	int identify(const T &value) {
		if (auto p = ids.find(value)) return *p;
		assert(ids.size() < std::size_t(std::numeric_limits<int>::max()));
		int id = int(ids.size());
		ids.set(value, id);
		return id;
	}
  public:
	OfflineRangeFrequency() = default;
	explicit OfflineRangeFrequency(const std::vector<T> &values) { build(values); }
	void build(const std::vector<T> &values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		ids.clear(); ids.reserve(values.size());
		initial.clear(); initial.reserve(values.size());
		for (const auto &value : values) initial.push_back(identify(value));
		current = initial;
		events.clear(); answers = 0;
	}
	int size() const { return int(initial.size()); }
	void reserve(int operations) { assert(operations >= 0); events.reserve(std::size_t(operations) * 2); }
	void set(int p, const T &value) {
		assert(0 <= p && p < size());
		int id = identify(value);
		if (current[p] == id) return;
		events.push_back({current[p], p, 0, 0, -1});
		events.push_back({id, p, 0, 0, 1});
		current[p] = id;
	}
	int add_query(int l, int r, const T &value) {
		assert(0 <= l && l <= r && r <= size() && answers < std::numeric_limits<int>::max());
		if (auto id = ids.find(value)) events.push_back({*id, l, r, answers, 0});
		return answers++;
	}
	std::vector<int> solve() const {
		std::vector<int> result(answers);
		if (!answers || std::none_of(events.begin(), events.end(), [](auto event) { return event.delta == 0; })) return result;
		if (std::none_of(events.begin(), events.end(), [](auto event) { return event.delta != 0; })) {
			std::vector<std::uint32_t> ends(std::size_t(size()) + 2);
			for (auto event : events) { ++ends[std::size_t(event.first) + 1]; ++ends[std::size_t(event.last) + 1]; }
			for (std::size_t i = 1; i < ends.size(); ++i) ends[i] += ends[i - 1];
			auto cursor = ends;
			std::vector<std::array<int, 2>> grouped(events.size() * 2);
			for (auto event : events) {
				grouped[cursor[event.first]++] = {event.value, -event.answer - 1};
				grouped[cursor[event.last]++] = {event.value, event.answer + 1};
			}
			std::vector<int> counts(ids.size());
			for (std::size_t r = 0; r <= initial.size(); ++r) {
				if (r) ++counts[initial[r - 1]];
				for (auto i = ends[r]; i < ends[r + 1]; ++i) {
					auto [value, signed_id] = grouped[i];
					if (signed_id > 0) result[signed_id - 1] += counts[value];
					else result[-signed_id - 1] -= counts[value];
				}
			}
			return result;
		}
		std::size_t m = ids.size();
		std::vector<std::size_t> starts(m + 1), initial_starts(m + 1), final_starts(m + 1);
		std::vector<unsigned char> queried(m);
		for (auto event : events) {
			++starts[event.value + 1];
			if (!event.delta) queried[event.value] = 1;
		}
		for (int value : initial) ++initial_starts[value + 1];
		for (int value : current) ++final_starts[value + 1];
		for (std::size_t i = 1; i <= m; ++i) {
			starts[i] += starts[i - 1];
			initial_starts[i] += initial_starts[i - 1];
			final_starts[i] += final_starts[i - 1];
		}
		std::vector<Event> grouped(events.size());
		std::vector<int> initial_positions(initial.size()), final_positions(initial.size());
		auto cursor = starts, before = initial_starts, after = final_starts;
		for (auto event : events) grouped[cursor[event.value]++] = event;
		for (int i = 0; i < size(); ++i) {
			initial_positions[before[initial[i]]++] = i;
			final_positions[after[current[i]]++] = i;
		}
		FenwickBitset frequencies(size());
		for (std::size_t value = 0; value < m; ++value) {
			if (!queried[value]) continue;
			for (auto i = initial_starts[value]; i < initial_starts[value + 1]; ++i) frequencies.set(initial_positions[i], true);
			for (auto i = starts[value]; i < starts[value + 1]; ++i) {
				auto event = grouped[i];
				if (event.delta) frequencies.set(event.first, event.delta > 0);
				else result[event.answer] = frequencies.count(event.first, event.last);
			}
			for (auto i = final_starts[value]; i < final_starts[value + 1]; ++i) frequencies.set(final_positions[i], false);
		}
		return result;
	}
};

#endif
