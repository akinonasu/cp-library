#ifndef SCC_CORE_HPP
#define SCC_CORE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace scc_detail {
inline std::pair<int, std::vector<int>> ids(int n, std::span<const std::pair<int, int>> edges) {
	assert(n >= 0 && edges.size() <= std::size_t(std::numeric_limits<int>::max()));
	std::vector<int> answer(n, -1);
	unsigned monotone = 3;
	for (auto [from, to] : edges) {
		assert(0 <= from && from < n && 0 <= to && to < n);
		monotone &= (from <= to ? 1U : 0U) | (from >= to ? 2U : 0U);
		if (!monotone) break;
	}
	if (monotone) {
		for (int v = 0; v < n; ++v) answer[v] = monotone & 1 ? v : n - 1 - v;
		return {n, std::move(answer)};
	}
	std::vector<int> offset(std::size_t(n) + 1), degree(n);
	for (auto [from, to] : edges) {
		assert(0 <= from && from < n && 0 <= to && to < n);
		++offset[from + 1];
		++degree[to];
	}
	bool functional = true;
	for (int v = 0; v < n; ++v) {
		functional &= offset[v + 1] <= 1;
		offset[v + 1] += offset[v];
	}
	std::vector<int> adjacency(edges.size()), work(offset.begin(), offset.end() - 1);
	for (auto [from, to] : edges) adjacency[work[from]++] = to;
	work.clear();
	for (int v = 0; v < n; ++v) if (!degree[v]) work.push_back(v);
	for (std::size_t head = 0; head < work.size(); ++head) {
		int v = work[head];
		answer[v] = static_cast<int>(head);
		for (int j = offset[v]; j < offset[v + 1]; ++j)
			if (--degree[adjacency[j]] == 0) work.push_back(adjacency[j]);
	}
	int peeled = static_cast<int>(work.size());
	if (peeled == n) return {n, std::move(answer)};
	if (functional) {
		int count = peeled;
		for (int v = 0; v < n; ++v) if (degree[v]) {
			int u = v;
			do {
				answer[u] = count;
				degree[u] = 0;
				u = adjacency[offset[u]];
			} while (u != v);
			++count;
		}
		return {count, std::move(answer)};
	}
	for (int v : work) answer[v] = -answer[v] - 1;
	for (int &x : degree) x = x ? -1 : n;
	work.clear();
	struct Frame { int vertex, next, low; };
	std::vector<Frame> stack;
	stack.reserve(n - peeled);
	int clock = 0, count = 0;
	auto enter = [&](int v) {
		degree[v] = clock++;
		work.push_back(v);
		stack.push_back({v, offset[v], degree[v]});
	};
	for (int root = 0; root < n; ++root) if (degree[root] == -1) {
		enter(root);
		while (!stack.empty()) {
			auto &frame = stack.back();
			int v = frame.vertex;
			int next = frame.next, low = frame.low, end = offset[v + 1];
			for (; next < end; ++next) {
				int to = adjacency[next];
				if (degree[to] == -1) break;
				low = std::min(low, degree[to]);
			}
			frame.low = low;
			if (next < end) {
				frame.next = next + 1;
				enter(adjacency[next]);
			} else {
				stack.pop_back();
				if (low == degree[v]) {
					int u;
					do {
						u = work.back();
						work.pop_back();
						degree[u] = n;
						answer[u] = count;
					} while (u != v);
					++count;
				}
				if (!stack.empty()) stack.back().low = std::min(stack.back().low, low);
			}
		}
	}
	for (int &x : answer) x = x < 0 ? -x - 1 : peeled + count - 1 - x;
	return {peeled + count, std::move(answer)};
}
}

#endif
