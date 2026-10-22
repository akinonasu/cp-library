#ifndef PROJECT_SELECTION_HPP
#define PROJECT_SELECTION_HPP

#include "max_flow.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template<class Cost = std::int64_t> class ProjectSelection {
	static_assert(std::is_integral_v<Cost> && std::is_signed_v<Cost> && sizeof(Cost) <= 8);
	using Wide = __int128_t;
	using Cap = std::uint64_t;
	struct Edge { int from, to; Cap cost; };
	int n_;
	Wide constant_ = 0;
	std::vector<Wide> unary_;
	std::vector<bool> flipped_;
	std::vector<Edge> edges_;
	std::vector<std::pair<int, int>> hard_;
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < n_); }
	bool orientation(int v) const { return !flipped_.empty() && flipped_[v]; }
	void unary(int v, Wide c0, Wide c1) {
		vertex(v);
		if (orientation(v)) std::swap(c0, c1);
		constant_ += c0;
		unary_[v] += c1 - c0;
	}
	static Cap capacity(Wide cost) {
		assert(0 <= cost && cost <= std::numeric_limits<Cap>::max());
		return static_cast<Cap>(cost);
	}
	void penalty(int from, int to, Wide cost) {
		assert(cost >= 0);
		if (cost && from != to) edges_.push_back({from, to, capacity(cost)});
	}
	void pair(int u, int v, std::array<Wide, 4> cost) {
		vertex(u), vertex(v);
		if (u == v) { unary(u, cost[0], cost[3]); return; }
		if (orientation(u)) std::swap(cost[0], cost[2]), std::swap(cost[1], cost[3]);
		if (orientation(v)) std::swap(cost[0], cost[1]), std::swap(cost[2], cost[3]);
		Wide delta = cost[1] + cost[2] - cost[0] - cost[3];
		assert(delta >= 0);
		constant_ += cost[0];
		unary_[u] += cost[3] - cost[1];
		unary_[v] += cost[1] - cost[0];
		penalty(u, v, delta);
	}
	void condition(int u, int v, bool u_value, bool v_value, Wide cost) {
		vertex(u), vertex(v);
		if (u == v) {
			if (u_value == v_value) unary(u, u_value ? 0 : cost, u_value ? cost : 0);
			return;
		}
		if (cost >= 0) {
			u_value = u_value != orientation(u), v_value = v_value != orientation(v);
			assert(!cost || u_value != v_value);
			penalty(u_value ? u : v, u_value ? v : u, cost);
			return;
		}
		std::array<Wide, 4> table{};
		table[2 * int(u_value) + int(v_value)] = cost;
		pair(u, v, table);
	}
	int auxiliary() {
		assert(unary_.size() < std::size_t(std::numeric_limits<int>::max() / 2 - 3));
		int v = static_cast<int>(unary_.size());
		unary_.push_back(0);
		return v;
	}

  public:
	using cost_type = Cost;
	using total_type = Wide;
	struct Result {
		Wide cost;
		std::vector<bool> selected;
		Wide profit() const { return -cost; }
	};
	explicit ProjectSelection(int n = 0) : n_(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2 - 3);
		unary_.resize(n);
	}
	ProjectSelection(int n, const std::vector<bool> &flipped) : ProjectSelection(n) {
		assert(flipped.size() == std::size_t(n));
		flipped_ = flipped;
	}
	int size() const { return n_; }
	bool is_flipped(int v) const { vertex(v); return orientation(v); }
	void add_constant(Cost cost) { constant_ += Wide(cost); }
	void add_cost(int v, Cost cost) { unary(v, 0, Wide(cost)); }
	void add_cost(int v, Cost false_cost, Cost true_cost) { unary(v, Wide(false_cost), Wide(true_cost)); }
	void add_profit(int v, Cost profit) { unary(v, 0, -Wide(profit)); }
	void add_profit(int v, Cost false_profit, Cost true_profit) { unary(v, -Wide(false_profit), -Wide(true_profit)); }
	void add_cost_if(int v, bool value, Cost cost) { unary(v, value ? 0 : Wide(cost), value ? Wide(cost) : 0); }
	void add_profit_if(int v, bool value, Cost profit) { unary(v, value ? 0 : -Wide(profit), value ? -Wide(profit) : 0); }
	void add_cost_if(int u, int v, bool u_value, bool v_value, Cost cost) {
		condition(u, v, u_value, v_value, Wide(cost));
	}
	void add_profit_if(int u, int v, bool u_value, bool v_value, Cost profit) {
		condition(u, v, u_value, v_value, -Wide(profit));
	}
	void add_penalty(int u, int v, Cost cost) {
		assert(cost >= 0);
		add_cost_if(u, v, true, false, cost);
	}
	void add_difference_cost(int u, int v, Cost cost) {
		vertex(u), vertex(v);
		assert(cost >= 0 && (u == v || !cost || orientation(u) == orientation(v)));
		penalty(u, v, Wide(cost));
		penalty(v, u, Wide(cost));
	}
	void add_same_cost(int u, int v, Cost cost) {
		vertex(u), vertex(v);
		assert(cost >= 0);
		if (u == v) { constant_ += Wide(cost); return; }
		assert(!cost || orientation(u) != orientation(v));
		penalty(u, v, Wide(cost));
		penalty(v, u, Wide(cost));
	}
	void add_same_profit(int u, int v, Cost profit) {
		assert(profit >= 0);
		add_difference_cost(u, v, profit);
		constant_ -= Wide(profit);
	}
	void add_difference_profit(int u, int v, Cost profit) {
		assert(profit >= 0);
		add_same_cost(u, v, profit);
		constant_ -= Wide(profit);
	}
	void add_pair_cost(int u, int v, Cost c00, Cost c01, Cost c10, Cost c11) {
		pair(u, v, {Wide(c00), Wide(c01), Wide(c10), Wide(c11)});
	}
	void add_pair_profit(int u, int v, Cost p00, Cost p01, Cost p10, Cost p11) {
		pair(u, v, {-Wide(p00), -Wide(p01), -Wide(p10), -Wide(p11)});
	}
	void add_all_profit(const std::vector<int> &vertices, bool value, Cost profit) {
		assert(profit >= 0);
		for ([[maybe_unused]] int v : vertices) vertex(v);
		if (!profit) return;
		if (vertices.empty()) { constant_ -= Wide(profit); return; }
		if (vertices.size() == 1) {
			add_profit_if(vertices[0], value, profit);
			return;
		}
		for ([[maybe_unused]] int v : vertices) assert(orientation(v) == orientation(vertices[0]));
		value = value != orientation(vertices[0]);
		int aux = auxiliary();
		if (value) unary_[aux] -= Wide(profit);
		else constant_ -= Wide(profit), unary_[aux] += Wide(profit);
		for (int v : vertices) {
			if (value) penalty(aux, v, Wide(profit));
			else penalty(v, aux, Wide(profit));
		}
	}
	void add_any_cost(const std::vector<int> &vertices, bool value, Cost cost) {
		assert(cost >= 0);
		add_all_profit(vertices, !value, cost);
		constant_ += Wide(cost);
	}
	void force(int v, bool value) {
		vertex(v);
		value = value != orientation(v);
		hard_.emplace_back(value ? -1 : v, value ? v : -2);
	}
	void forbid(int u, int v, bool u_value, bool v_value) {
		vertex(u), vertex(v);
		if (u == v) {
			if (u_value == v_value) force(u, !u_value);
			return;
		}
		u_value = u_value != orientation(u), v_value = v_value != orientation(v);
		assert(u_value != v_value);
		hard_.emplace_back(u_value ? u : v, u_value ? v : u);
	}
	void imply(int u, int v) { forbid(u, v, true, false); }
	void imply(int u, int v, bool u_value, bool v_value) { forbid(u, v, u_value, !v_value); }
	void same(int u, int v) { imply(u, v); imply(v, u); }
	void different(int u, int v) { forbid(u, v, false, false); forbid(u, v, true, true); }
	void at_most_one(int u, int v) { forbid(u, v, true, true); }
	void at_least_one(int u, int v) { forbid(u, v, false, false); }
	std::optional<Result> solve() const {
		Wide bound = 0, base = constant_;
		std::size_t count = edges_.size() + hard_.size();
		for (const auto &edge : edges_) bound += Wide(edge.cost);
		for (Wide cost : unary_) {
			bound += cost < 0 ? -cost : cost;
			count += cost != 0;
			if (cost < 0) base += cost;
		}
		assert(bound < Wide(std::numeric_limits<Cap>::max()));
		const Cap infinity = capacity(bound + 1);
		const int source = static_cast<int>(unary_.size()), sink = source + 1;
		MaxFlow<Cap> graph(source + 2);
		graph.reserve(count);
		auto endpoint = [&](int v) { return v == -1 ? source : v == -2 ? sink : v; };
		for (auto [u, v] : hard_) graph.add_edge(endpoint(u), endpoint(v), infinity);
		if (!hard_.empty() && graph.min_cut(source)[sink]) return std::nullopt;
		for (const auto &edge : edges_) graph.add_edge(edge.from, edge.to, edge.cost);
		for (int v = 0; v < source; ++v) {
			if (unary_[v] < 0) graph.add_edge(source, v, capacity(-unary_[v]));
			else if (unary_[v] > 0) graph.add_edge(v, sink, capacity(unary_[v]));
		}
		Wide cost = base + Wide(graph.flow(source, sink));
		auto selected = graph.min_cut(source);
		selected.resize(n_);
		if (!flipped_.empty()) for (int v = 0; v < n_; ++v) selected[v] = selected[v] != flipped_[v];
		return Result{cost, std::move(selected)};
	}
};

#endif
