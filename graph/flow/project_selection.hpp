#ifndef PROJECT_SELECTION_HPP
#define PROJECT_SELECTION_HPP

#include "max_flow.hpp"
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
	std::vector<Edge> edges_;
	std::vector<std::pair<int, int>> hard_;
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < n_); }
	static Cap capacity(Wide cost) {
		assert(0 <= cost && cost <= std::numeric_limits<Cap>::max());
		return static_cast<Cap>(cost);
	}
	void penalty(int from, int to, Wide cost) {
		assert(cost >= 0);
		if (cost && from != to) edges_.push_back({from, to, capacity(cost)});
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
	int size() const { return n_; }
	void add_constant(Cost cost) { constant_ += Wide(cost); }
	void add_cost(int v, Cost cost) { vertex(v); unary_[v] += Wide(cost); }
	void add_cost(int v, Cost false_cost, Cost true_cost) {
		vertex(v);
		constant_ += Wide(false_cost);
		unary_[v] += Wide(true_cost) - Wide(false_cost);
	}
	void add_profit(int v, Cost profit) { vertex(v); unary_[v] -= Wide(profit); }
	void add_profit(int v, Cost false_profit, Cost true_profit) {
		vertex(v);
		constant_ -= Wide(false_profit);
		unary_[v] += Wide(false_profit) - Wide(true_profit);
	}
	void add_penalty(int u, int v, Cost cost) {
		vertex(u), vertex(v);
		penalty(u, v, Wide(cost));
	}
	void add_difference_cost(int u, int v, Cost cost) {
		vertex(u), vertex(v);
		penalty(u, v, Wide(cost));
		penalty(v, u, Wide(cost));
	}
	void add_pair_cost(int u, int v, Cost c00, Cost c01, Cost c10, Cost c11) {
		vertex(u), vertex(v);
		if (u == v) { add_cost(u, c00, c11); return; }
		Wide cost = Wide(c01) + Wide(c10) - Wide(c00) - Wide(c11);
		assert(cost >= 0);
		constant_ += Wide(c00);
		unary_[u] += Wide(c11) - Wide(c01);
		unary_[v] += Wide(c01) - Wide(c00);
		penalty(u, v, cost);
	}
	void add_all_profit(const std::vector<int> &vertices, bool value, Cost profit) {
		assert(profit >= 0);
		for ([[maybe_unused]] int v : vertices) vertex(v);
		if (!profit) return;
		if (vertices.empty()) { constant_ -= Wide(profit); return; }
		if (vertices.size() == 1) {
			add_profit(vertices[0], value ? Cost(0) : profit, value ? profit : Cost(0));
			return;
		}
		int aux = auxiliary();
		if (value) unary_[aux] -= Wide(profit);
		else constant_ -= Wide(profit), unary_[aux] += Wide(profit);
		for (int v : vertices) {
			if (value) penalty(aux, v, Wide(profit));
			else penalty(v, aux, Wide(profit));
		}
	}
	void force(int v, bool value) {
		vertex(v);
		hard_.emplace_back(value ? -1 : v, value ? v : -2);
	}
	void imply(int u, int v) {
		vertex(u), vertex(v);
		if (u != v) hard_.emplace_back(u, v);
	}
	void same(int u, int v) { imply(u, v); imply(v, u); }
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
		return Result{cost, std::move(selected)};
	}
};

#endif
