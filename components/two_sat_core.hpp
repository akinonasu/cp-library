#ifndef TWO_SAT_CORE_HPP
#define TWO_SAT_CORE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace two_sat_detail {
inline bool solve(int variables, std::span<const std::pair<int, int>> edges, std::span<const int> forced, std::vector<bool> &answer) {
	assert(0 <= variables && variables <= std::numeric_limits<int>::max() / 2);
	assert(edges.size() <= std::size_t(std::numeric_limits<int>::max()) && answer.size() == std::size_t(variables));
	int n = 2 * variables;
	std::vector<signed char> values(variables, -1);
	std::vector<int> trail;
	trail.reserve(variables);
	for (int literal : forced) {
		if (values[literal >> 1] < 0) {
			values[literal >> 1] = literal & 1;
			trail.push_back(literal);
		} else if (values[literal >> 1] != (literal & 1)) return false;
	}
	if (trail.size() == std::size_t(variables)) {
		for (auto [from, to] : edges) {
			if (values[from >> 1] == (from & 1) && values[to >> 1] != (to & 1)) return false;
		}
		for (int i = 0; i < variables; ++i) answer[i] = values[i];
		return true;
	}
	std::vector<int> offset(std::size_t(n) + 1, -1), adjacency(edges.size());
	std::size_t branching = 0;
	for (std::size_t i = 0; i < edges.size(); ++i) {
		auto [from, to] = edges[i];
		branching += offset[from] >= 0;
		adjacency[i] = offset[from];
		offset[from] = static_cast<int>(i);
	}
	bool compact = false;
	auto make_compact = [&] {
		std::fill(offset.begin(), offset.end(), 0);
		for (auto [from, to] : edges) ++offset[from];
		for (int v = 1; v < n; ++v) offset[v] += offset[v - 1];
		offset[n] = static_cast<int>(edges.size());
		for (auto [from, to] : edges) adjacency[--offset[from]] = to;
		compact = true;
	};
	if (variables >= 1024) {
		auto closure = [&]<bool Adaptive>() -> int {
			std::size_t budget = std::size_t(variables) + edges.size();
			if constexpr (!Adaptive) budget = (std::min)(budget, std::numeric_limits<std::size_t>::max() / 2) * 2;
			std::size_t switch_after = std::max<std::size_t>(1024, (std::size_t(variables) + edges.size()) / 16);
			std::size_t compact_budget = budget - std::min(budget, switch_after);
			auto propagate = [&] {
				for (std::size_t i = 0; i < trail.size(); ++i) {
					if (!budget) return -1;
					--budget;
					int v = trail[i];
					if constexpr (Adaptive) if (!compact && budget < compact_budget) make_compact();
					auto visit = [&](int to) {
						if (!budget) return -1;
						--budget;
						if (values[to >> 1] < 0) {
							values[to >> 1] = to & 1;
							trail.push_back(to);
						} else if (values[to >> 1] != (to & 1)) {
							for (int literal : trail) values[literal >> 1] = -1;
							return 0;
						}
						return 1;
					};
					if constexpr (Adaptive) if (compact) {
						for (int at = offset[v]; at < offset[v + 1]; ++at) {
							int status = visit(adjacency[at]);
							if (status != 1) return status;
						}
						continue;
					}
					for (int at = offset[v]; at >= 0; at = adjacency[at]) {
						int status = visit(edges[at].second);
						if (status != 1) return status;
					}
				}
				return 1;
			};
			auto empty = [&](int v) {
				if constexpr (Adaptive) if (compact) return offset[v] == offset[v + 1];
				return offset[v] < 0;
			};
			int status = propagate();
			if (!status) return 0;
			for (int i = 0; status > 0 && i < variables; ++i) if (values[i] < 0) {
				if (empty(2 * i)) values[i] = 0;
				else if (empty(2 * i + 1)) values[i] = 1;
				else {
					trail.assign(1, 2 * i);
					values[i] = 0;
					status = propagate();
					if (!status) {
						trail.assign(1, 2 * i + 1);
						values[i] = 1;
						status = propagate();
						if (!status) return 0;
					}
				}
			}
			return status;
		};
		int status = edges.size() >= std::size_t(variables) && branching > edges.size() / 32
			? closure.template operator()<true>() : closure.template operator()<false>();
		if (status >= 0) {
			if (status) for (int i = 0; i < variables; ++i) answer[i] = values[i];
			return status;
		}
	}
	if (!compact) make_compact();
	trail.clear();
	std::vector<int> order(n, -1);
	struct Frame { int vertex, next, low; };
	std::vector<Frame> stack;
	stack.reserve(n);
	int clock = 0, component = -2;
	auto enter = [&](int v) {
		order[v] = clock++;
		trail.push_back(v);
		stack.push_back({v, offset[v], order[v]});
	};
	for (int root = 0; root < n; ++root) if (order[root] == -1) {
		if (offset[root] == offset[root + 1]) { order[root] = component--; continue; }
		enter(root);
		while (!stack.empty()) {
			auto &frame = stack.back();
			int v = frame.vertex, next = frame.next, low = frame.low;
			for (; next < offset[v + 1]; ++next) {
				int to = adjacency[next];
				if (order[to] == -1) {
					if (offset[to] == offset[to + 1]) { order[to] = component--; continue; }
					break;
				}
				if (order[to] >= 0) low = std::min(low, order[to]);
			}
			frame.low = low;
			if (next < offset[v + 1]) {
				frame.next = next + 1;
				enter(adjacency[next]);
			} else {
				stack.pop_back();
				if (low == order[v]) {
					int u;
					do {
						u = trail.back();
						trail.pop_back();
						order[u] = component;
						if (order[u ^ 1] == component) return false;
					} while (u != v);
					--component;
				}
				if (!stack.empty()) stack.back().low = std::min(stack.back().low, low);
			}
		}
	}
	for (int i = 0; i < variables; ++i) answer[i] = order[2 * i] < order[2 * i + 1];
	return true;
}
}

#endif
