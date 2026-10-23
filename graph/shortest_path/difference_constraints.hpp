#ifndef DIFFERENCE_CONSTRAINTS_HPP
#define DIFFERENCE_CONSTRAINTS_HPP

#include "bellman_ford.hpp"
#include "dijkstra.hpp"
#include <optional>

template<class Cost = std::int64_t>
class DifferenceConstraints {
	static_assert(std::numeric_limits<Cost>::is_integer && std::numeric_limits<Cost>::is_signed && sizeof(Cost) <= 16);

  public:
	using value_type = std::conditional_t<(sizeof(Cost) <= 8), std::int64_t, __int128_t>;
	using Constraint = DirectedCSR<value_type>::Edge;
	class Result {
		friend class DifferenceConstraints;
		int n_;
		DirectedCSR<value_type> graph_;
		std::vector<value_type> values_;
		std::vector<Constraint> negative_cycle_;
		explicit Result(int n) : n_(n) {}
		void check([[maybe_unused]] int v) const { assert(0 <= v && v <= n_); }
		value_type potential(int v) const { return v == n_ ? 0 : values_[v]; }
		template<class Function>
		auto with_distances(int source, Function &&function) const {
			assert(feasible());
			check(source);
			if (!n_) return function(std::vector<value_type>{0});
			auto distance = dijkstra(graph_, source, std::numeric_limits<value_type>::max());
			if (std::find(distance.begin(), distance.end(), value_type(-1)) != distance.end())
				for (int v = 0; v <= n_; ++v) if (distance[v] != -1)
					for (auto [to, weight] : graph_[v])
						if (distance[to] == -1) throw std::overflow_error("DifferenceConstraints distance");
			return function(distance);
		}

	  public:
		Result(const Result &) = default;
		Result &operator=(const Result &) = default;
		Result(Result &&other) noexcept : n_(std::exchange(other.n_, 0)), graph_(std::move(other.graph_)),
			values_(std::move(other.values_)), negative_cycle_(std::move(other.negative_cycle_)) {}
		Result &operator=(Result &&other) noexcept {
			if (this != &other) {
				n_ = std::exchange(other.n_, 0);
				graph_ = std::move(other.graph_);
				values_ = std::move(other.values_);
				negative_cycle_ = std::move(other.negative_cycle_);
			}
			return *this;
		}
		int size() const { return n_; }
		int origin() const { return n_; }
		bool feasible() const { return negative_cycle_.empty(); }
		const std::vector<value_type> &values() const { assert(feasible()); return values_; }
		value_type value(int v) const { assert(feasible()); check(v); return potential(v); }
		const std::vector<Constraint> &negative_cycle() const { return negative_cycle_; }
		std::vector<std::optional<value_type>> upper_bounds(int source) const {
			return with_distances(source, [&](const auto &reduced) {
				std::vector<std::optional<value_type>> result(n_);
				for (int v = 0; v < n_; ++v)
					if (reduced[v] != -1) result[v] = add_sub(reduced[v], potential(v), potential(source));
				return result;
			});
		}
		std::vector<std::optional<value_type>> upper_bounds() const { return upper_bounds(origin()); }
		std::optional<value_type> max_difference(int u, int v) const {
			assert(feasible());
			check(u), check(v);
			if (u == v) return 0;
			return with_distances(u, [&](const auto &reduced) -> std::optional<value_type> {
				if (reduced[v] == -1) return std::nullopt;
				return add_sub(reduced[v], potential(v), potential(u));
			});
		}
		std::optional<value_type> min_difference(int u, int v) const {
			auto result = max_difference(v, u);
			if (result) *result = checked_sub(0, *result);
			return result;
		}
		std::optional<value_type> upper_bound(int v) const { return max_difference(origin(), v); }
		std::optional<value_type> lower_bound(int v) const { return min_difference(origin(), v); }
	};

  private:
	int n_;
	std::vector<Constraint> constraints_;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v <= n_); }
	std::size_t add(int u, int v, value_type bound) {
		check(u), check(v);
		constraints_.push_back({u, v, bound});
		return constraints_.size() - 1;
	}
	static value_type checked_add(value_type a, value_type b) {
		value_type result;
		if (__builtin_add_overflow(a, b, &result)) throw std::overflow_error("DifferenceConstraints addition");
		return result;
	}
	static value_type checked_sub(value_type a, value_type b) {
		value_type result;
		if (__builtin_sub_overflow(a, b, &result)) throw std::overflow_error("DifferenceConstraints subtraction");
		return result;
	}
	static value_type add_sub(value_type a, value_type b, value_type c) {
		if ((b < 0) == (c < 0)) return checked_add(a, checked_sub(b, c));
		return checked_sub(checked_add(a, b), c);
	}

  public:
	explicit DifferenceConstraints(int n = 0) : n_(n) { assert(0 <= n && n <= std::numeric_limits<int>::max() - 2); }
	int size() const { return n_; }
	int origin() const { return n_; }
	std::size_t constraint_count() const { return constraints_.size(); }
	const std::vector<Constraint> &constraints() const { return constraints_; }
	void reserve(std::size_t count) { constraints_.reserve(count); }
	std::size_t add_le(int u, int v, Cost bound) { return add(u, v, value_type(bound)); }
	std::size_t add_ge(int u, int v, Cost bound) { return add(v, u, checked_sub(0, value_type(bound))); }
	std::size_t add_lt(int u, int v, Cost bound) { return add(u, v, checked_sub(value_type(bound), 1)); }
	std::size_t add_gt(int u, int v, Cost bound) { return add(v, u, checked_sub(-1, value_type(bound))); }
	std::pair<std::size_t, std::size_t> add_eq(int u, int v, Cost value) {
		value_type reverse = checked_sub(0, value_type(value));
		return {add(u, v, value), add(v, u, reverse)};
	}
	std::pair<std::size_t, std::size_t> add_range(int u, int v, Cost lower, Cost upper) {
		value_type reverse = checked_sub(0, value_type(lower));
		return {add(v, u, reverse), add(u, v, upper)};
	}
	std::size_t add_upper_bound(int v, Cost bound) { return add_le(origin(), v, bound); }
	std::size_t add_lower_bound(int v, Cost bound) { return add_ge(origin(), v, bound); }
	std::pair<std::size_t, std::size_t> fix(int v, Cost value) { return add_eq(origin(), v, value); }
	std::pair<std::size_t, std::size_t> add_range(int v, Cost lower, Cost upper) { return add_range(origin(), v, lower, upper); }
	Result solve() const {
		Result result(n_);
		std::vector<Constraint> edges = constraints_;
		edges.reserve(constraints_.size() + std::size_t(n_) + 1);
		const int source = n_ + 1;
		for (int v = 0; v <= n_; ++v) edges.push_back({source, v, 0});
		{
			DirectedCSR<value_type> graph(n_ + 2, edges);
			auto found = bellman_ford<value_type>(graph, source);
			if (found.has_negative_cycle()) {
				for (std::size_t i = 0; i < found.negative_cycle_edges.size(); ++i) {
					int from = found.negative_cycle[i];
					auto [to, weight] = graph[from][found.negative_cycle_edges[i]];
					assert(from <= n_ && to <= n_);
					result.negative_cycle_.push_back({from, to, weight});
				}
				return result;
			}
			result.values_.resize(n_);
			for (int v = 0; v < n_; ++v) result.values_[v] = checked_sub(found.distance[v], found.distance[n_]);
		}
		edges.resize(constraints_.size());
		for (auto &edge : edges) {
			edge.weight = add_sub(edge.weight, result.potential(edge.from), result.potential(edge.to));
			assert(edge.weight >= 0);
			if (edge.weight == std::numeric_limits<value_type>::max()) throw std::overflow_error("DifferenceConstraints reduced weight");
		}
		result.graph_ = DirectedCSR<value_type>(n_ + 1, edges);
		return result;
	}
};

#endif
