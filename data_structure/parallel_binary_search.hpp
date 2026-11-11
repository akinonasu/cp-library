#ifndef PARALLEL_BINARY_SEARCH_HPP
#define PARALLEL_BINARY_SEARCH_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

template<class Reset, class Apply, class Check>
std::vector<int> parallel_binary_search(int update_count, int query_count, Reset reset, Apply apply, Check check) {
	assert(update_count >= 0 && query_count >= 0);
	std::vector<int> answer(query_count, -1);
	if (!query_count) return answer;
	reset();
	std::vector<int> order;
	order.reserve(query_count);
	for (int id = 0; id < query_count; ++id) {
		if (check(id)) answer[id] = 0;
		else order.push_back(id);
	}
	if (order.empty() || !update_count) return answer;
	for (int i = 0; i < update_count; ++i) apply(i);
	std::size_t kept = 0;
	for (int id : order) if (check(id)) order[kept++] = id;
	order.resize(kept);
	if (order.empty()) return answer;
	if (update_count == 1) {
		for (int id : order) answer[id] = 1;
		return answer;
	}
	struct Group { int last, low, high; };
	std::vector<Group> groups, next;
	groups.reserve(std::min(order.size(), std::size_t(update_count)));
	next.reserve(groups.capacity());
	groups.push_back({int(order.size()), 0, update_count});
	std::vector<int> scratch(order.size());
	while (!groups.empty()) {
		reset();
		int time = 0, first = 0;
		bool pending = false;
		next.clear();
		for (const auto &g : groups) {
			int begin = first;
			first = g.last;
			if (g.high - g.low == 1) { next.push_back(g); continue; }
			const int middle = g.low + (g.high - g.low) / 2;
			while (std::uint32_t(time) < std::uint32_t(middle)) apply(time++);
			int left = begin, right = g.last;
			for (int i = begin; i < g.last; ++i) {
				int id = order[i];
				if (check(id)) scratch[left++] = id;
				else scratch[--right] = id;
			}
			std::reverse(scratch.begin() + right, scratch.begin() + g.last);
			if (g.low + 1 == middle) {
				for (int i = begin; i < left; ++i) answer[scratch[i]] = middle;
			} else if (left != begin) pending = true;
			if (left != begin) next.push_back({left, g.low, middle});
			if (middle + 1 == g.high) {
				for (int i = right; i < g.last; ++i) answer[scratch[i]] = g.high;
			} else if (right != g.last) pending = true;
			if (right != g.last) next.push_back({g.last, middle, g.high});
		}
		if (!pending) break;
		groups.swap(next);
		order.swap(scratch);
	}
	return answer;
}

#endif
