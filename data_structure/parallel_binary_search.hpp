#ifndef PARALLEL_BINARY_SEARCH_HPP
#define PARALLEL_BINARY_SEARCH_HPP

#include "../components/radix_sort.hpp"
#include <cassert>
#include <cstdint>

template<class Reset, class Apply, class Check>
std::vector<int> parallel_binary_search(int update_count, int query_count, Reset reset, Apply apply, Check check) {
	assert(update_count >= 0 && query_count >= 0);
	std::vector<int> answer(query_count, -1);
	if (!query_count) return answer;
	reset();
	std::vector<int> active;
	active.reserve(query_count);
	for (int id = 0; id < query_count; ++id) {
		if (check(id)) answer[id] = 0;
		else active.push_back(id);
	}
	if (active.empty() || !update_count) return answer;
	for (int i = 0; i < update_count; ++i) apply(i);
	std::size_t kept = 0;
	for (int id : active) if (check(id)) active[kept++] = id;
	active.resize(kept);
	if (active.empty()) return answer;
	std::vector<int> low(query_count), high(query_count, update_count);
	std::vector<std::pair<std::uint32_t, int>> middle;
	middle.reserve(active.size());
	for (;;) {
		middle.clear();
		for (int id : active) if (high[id] - low[id] > 1)
			middle.emplace_back(std::uint32_t(low[id] + (high[id] - low[id]) / 2), id);
		if (middle.empty()) break;
		stable_radix_sort(middle, [](const auto &entry) { return entry.first; });
		reset();
		int time = 0;
		for (auto [target, id] : middle) {
			while (std::uint32_t(time) < target) apply(time++);
			if (check(id)) high[id] = time;
			else low[id] = time;
		}
	}
	for (int id : active) answer[id] = high[id];
	return answer;
}

#endif
