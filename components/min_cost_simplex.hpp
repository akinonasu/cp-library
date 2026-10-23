#ifndef MIN_COST_SIMPLEX_HPP
#define MIN_COST_SIMPLEX_HPP

#include "flow_graph.hpp"
#include <cmath>
#include <cstdint>
#include <optional>

namespace flow_detail {
template<class Cap> class NetworkSimplex {
	using Cost = __int128_t;
	struct Edge { int to; Cap cap; Cost cost; };
	struct Parent { int vertex, edge; Cap up, down; };
	int n;
	std::vector<Edge> edges;
	std::vector<Cost> potential;
	std::uint64_t work = 0, budget;
	bool solve() {
		const int original_edge_count = int(edges.size());
		potential.assign(n + 1, Cost(0));
		Cost artificial_cost = 1;
		for (int edge = 0; edge < original_edge_count; edge += 2)
			artificial_cost += edges[edge].cost < 0 ? -edges[edge].cost : edges[edge].cost;

		std::vector<Parent> parent(n);
		edges.reserve(original_edge_count + 2 * n);

		for (int vertex = 0; vertex < n; ++vertex) {
			edges.push_back(Edge{n, 0, artificial_cost});
			edges.push_back(Edge{vertex, 0, -artificial_cost});
			potential[vertex] = -artificial_cost;
			parent[vertex] = Parent{n, int(edges.size()) - 2, 0, 0};
		}

		std::vector<int> depth(n + 1, 1);
		depth[n] = 0;
		std::vector<int> next(2 * (n + 1));
		std::vector<int> previous(2 * (n + 1));
		auto connect = [&](int first, int second) {
			next[first] = second;
			previous[second] = first;
		};
		for (int vertex = 0; vertex <= n; ++vertex) {
			connect(2 * vertex, 2 * vertex + 1);
		}
		for (int vertex = 0; vertex < n; ++vertex) {
			connect(2 * vertex + 1, next[2 * n]);
			connect(2 * n, 2 * vertex);
		}

		auto push_flow = [&](int entering_edge) {
			const int first = edges[entering_edge ^ 1].to;
			const int second = edges[entering_edge].to;
			const Cost cycle_cost =
				edges[entering_edge].cost
				+ potential[first] - potential[second];

			Cap amount = edges[entering_edge].cap;
			bool leave_first_side = true;
			int leaving_vertex = second;

			int first_ancestor = first;
			int second_ancestor = second;
			auto move_first_up = [&] {
				if (++work > budget) return false;
				if (parent[first_ancestor].down < amount) {
					amount = parent[first_ancestor].down;
					leaving_vertex = first_ancestor;
					leave_first_side = true;
				}
				first_ancestor = parent[first_ancestor].vertex;
				return true;
			};
			auto move_second_up = [&] {
				if (++work > budget) return false;
				if (parent[second_ancestor].up <= amount) {
					amount = parent[second_ancestor].up;
					leaving_vertex = second_ancestor;
					leave_first_side = false;
				}
				second_ancestor = parent[second_ancestor].vertex;
				return true;
			};
			if (depth[first_ancestor] >= depth[second_ancestor]) {
				int difference = depth[first_ancestor] - depth[second_ancestor];
				for (int i = 0; i < difference; ++i) if (!move_first_up()) return false;
			} else {
				int difference = depth[second_ancestor] - depth[first_ancestor];
				for (int i = 0; i < difference; ++i) if (!move_second_up()) return false;
			}
			while (first_ancestor != second_ancestor) {
				if (!move_first_up() || !move_second_up()) return false;
			}
			const int ancestor = first_ancestor;

			if (amount != Cap(0)) {
				int vertex = first;
				while (vertex != ancestor) {
					if (++work > budget) return false;
					parent[vertex].up += amount;
					parent[vertex].down -= amount;
					vertex = parent[vertex].vertex;
				}
				vertex = second;
				while (vertex != ancestor) {
					if (++work > budget) return false;
					parent[vertex].up -= amount;
					parent[vertex].down += amount;
					vertex = parent[vertex].vertex;
				}
			}

			int vertex = first;
			int new_parent = second;
			std::pair<Cap, Cap> parent_capacities{
				edges[entering_edge].cap - amount,
				edges[entering_edge ^ 1].cap + amount
			};
			Cost potential_difference = -cycle_cost;
			if (!leave_first_side) {
				std::swap(vertex, new_parent);
				std::swap(parent_capacities.first, parent_capacities.second);
				potential_difference = -potential_difference;
			}
			int parent_edge = entering_edge ^ (leave_first_side ? 0 : 1);

			while (new_parent != leaving_vertex) {
				if (++work > budget) return false;
				int new_depth = depth[new_parent];
				int tour_index = 2 * vertex;
				while (tour_index != 2 * vertex + 1) {
					if (++work > budget) return false;
					if ((tour_index & 1) == 0) {
						new_depth++;
						potential[tour_index / 2] += potential_difference;
						depth[tour_index / 2] = new_depth;
					} else {
						new_depth--;
					}
					tour_index = next[tour_index];
				}

				connect(previous[2 * vertex], next[2 * vertex + 1]);
				connect(2 * vertex + 1, next[2 * new_parent]);
				connect(2 * new_parent, 2 * vertex);

				std::swap(parent[vertex].edge, parent_edge);
				parent_edge ^= 1;
				std::swap(parent[vertex].up, parent_capacities.first);
				std::swap(parent[vertex].down, parent_capacities.second);
				std::swap(parent_capacities.first, parent_capacities.second);

				int old_parent = parent[vertex].vertex;
				parent[vertex].vertex = new_parent;
				new_parent = vertex;
				vertex = old_parent;
			}
			edges[parent_edge].cap = parent_capacities.first;
			edges[parent_edge ^ 1].cap = parent_capacities.second;
			return true;
		};

		const int candidate_limit = std::max(
			int(0.2 * std::sqrt(double(original_edge_count))), 10
		);
		const int minor_limit = std::max(candidate_limit / 10, 3);
		std::vector<int> candidates;
		candidates.reserve(candidate_limit);

		auto minor_pivot = [&] {
			Cost best_cost = Cost(0);
			int best_edge = -1;
			int index = 0;
			while (index < int(candidates.size())) {
				if (++work > budget) return false;
				int edge = candidates[index];
				if (edges[edge].cap == Cap(0)) {
					candidates[index] = candidates.back();
					candidates.pop_back();
					continue;
				}
				Cost reduced_cost =
					edges[edge].cost
					+ potential[edges[edge ^ 1].to]
					- potential[edges[edge].to];
				if (reduced_cost >= Cost(0)) {
					candidates[index] = candidates.back();
					candidates.pop_back();
					continue;
				}
				if (reduced_cost < best_cost) {
					best_cost = reduced_cost;
					best_edge = edge;
				}
				++index;
			}
			if (best_edge == -1) return false;
			return push_flow(best_edge);
		};

		int edge = 0;
		while (true) {
			for (int iteration = 0; iteration < minor_limit; ++iteration) {
				if (++work > budget) return false;
				if (!minor_pivot()) break;
			}

			if (work > budget) return false;
			Cost best_cost = Cost(0);
			int best_edge = -1;
			candidates.clear();
			for (int scanned = 0; scanned < int(edges.size()); ++scanned) {
				if (++work > budget) return false;
				if (edges[edge].cap != Cap(0)) {
					Cost reduced_cost =
						edges[edge].cost
						+ potential[edges[edge ^ 1].to]
						- potential[edges[edge].to];
					if (reduced_cost < Cost(0)) {
						if (reduced_cost < best_cost) {
							best_cost = reduced_cost;
							best_edge = edge;
						}
						candidates.push_back(edge);
						if (int(candidates.size()) == candidate_limit) break;
					}
				}
				++edge;
				if (edge == int(edges.size())) edge = 0;
			}
			if (candidates.empty()) break;
			if (!push_flow(best_edge)) return false;
		}

		for (int vertex = 0; vertex < n; ++vertex) {
			edges[parent[vertex].edge].cap = parent[vertex].up;
			edges[parent[vertex].edge ^ 1].cap = parent[vertex].down;
		}

		return true;
	}

public:
	template<class Cost>
	NetworkSimplex(const Graph<Cap>& graph, const std::vector<Cost>& costs, int s, int t, Cap limit, std::uint64_t budget)
		: n(graph.n), budget(budget) {
		edges.reserve(graph.arcs.size() + std::size_t(n) * 2 + 2);
		__int128_t maximum = 0;
		for (std::size_t i = 0; i < graph.arcs.size(); ++i) {
			edges.push_back({graph.arcs[i].to, graph.arcs[i].cap, __int128_t(costs[i])});
			maximum = std::max(maximum, costs[i] < 0 ? -__int128_t(costs[i]) : __int128_t(costs[i]));
		}
		edges.push_back({s, limit, -__int128_t(n) * maximum - 1});
		edges.push_back({t, 0, __int128_t(n) * maximum + 1});
	}
	template<class Cost, class Total>
	std::optional<std::pair<Cap, Total>> run(Graph<Cap>& graph, const std::vector<Cost>& costs) {
		if (!solve()) return std::nullopt;
		Total total = 0;
		for (std::size_t i = 0; i < graph.arcs.size(); i += 2) {
			total += Total((__int128_t(edges[i ^ 1].cap) - __int128_t(graph.arcs[i ^ 1].cap)) * __int128_t(costs[i]));
			graph.arcs[i].cap = edges[i].cap;
			graph.arcs[i ^ 1].cap = edges[i ^ 1].cap;
		}
		return std::pair<Cap, Total>{edges[graph.arcs.size() + 1].cap, total};
	}
};

template<class Cap, class Cost, class Total>
std::optional<std::pair<Cap, Total>> simplex_flow(Graph<Cap>& graph, const std::vector<Cost>& costs,
	int s, int t, Cap limit, std::uint64_t budget = 0) {
	if (graph.n > (std::numeric_limits<int>::max() - 2) / 2) return std::nullopt;
	if (graph.arcs.size() > std::size_t(std::numeric_limits<int>::max()) - std::size_t(graph.n) * 2 - 2) return std::nullopt;
	Cap upper = 0;
	for (int j = graph.start[s]; j < graph.start[s + 1]; ++j)
		upper += std::min<Cap>(limit - upper, graph.arcs[graph.order[j]].cap);
	if (!upper) return std::pair<Cap, Total>{0, 0};
	if (!budget) budget = std::max<std::uint64_t>(10000, std::min<std::uint64_t>(25000000, std::uint64_t(graph.n) * graph.arcs.size()) * 4);
	return NetworkSimplex<Cap>(graph, costs, s, t, upper, budget).template run<Cost, Total>(graph, costs);
}
}

#endif
