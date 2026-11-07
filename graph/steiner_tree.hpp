#ifndef STEINER_TREE_HPP
#define STEINER_TREE_HPP

#include "../components/dijkstra_queue.hpp"
#include "../components/undirected_csr.hpp"
#include "../data_structure/union_find.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

template<class Cost = std::int64_t, class Sum = Cost>
class SteinerTree {
	static_assert((std::numeric_limits<Cost>::is_integer || std::is_floating_point_v<Cost>) && !std::is_same_v<Cost, bool>);
	static_assert((std::numeric_limits<Sum>::is_integer || std::is_floating_point_v<Sum>) && !std::is_same_v<Sum, bool>);
	static_assert(std::numeric_limits<Cost>::is_integer == std::numeric_limits<Sum>::is_integer);
	static_assert(std::numeric_limits<Cost>::digits <= std::numeric_limits<Sum>::digits);
	int n_;

  public:
	struct Edge { int from, to; Cost cost; };
	struct Result { Sum cost = 0; std::vector<int> edges; };

  private:
	std::vector<Edge> edges_;
	template<bool Restore>
	std::optional<Result> run(std::vector<int> terminals, Sum inf) const {
		assert(inf > 0);
		for ([[maybe_unused]] int v : terminals) assert(0 <= v && v < n_);
		std::sort(terminals.begin(), terminals.end());
		terminals.erase(std::unique(terminals.begin(), terminals.end()), terminals.end());
		if (terminals.size() <= 1) return Result{};
		int root = terminals.back();
		terminals.pop_back();
		auto graph = [&] {
			std::vector<std::pair<int, int>> endpoints;
			endpoints.reserve(edges_.size());
			for (const auto &e : edges_) endpoints.emplace_back(e.from, e.to);
			return UndirectedCSR(n_, endpoints);
		}();
		{
			std::vector<unsigned char> seen(n_);
			std::vector<int> work{root};
			seen[root] = 1;
			for (std::size_t i = 0; i < work.size(); ++i)
				for (auto [u, id] : graph[work[i]]) if (!seen[u] && Sum(edges_[id].cost) < inf) {
					seen[u] = 1, work.push_back(u);
				}
			for (int v : terminals) if (!seen[v]) return std::nullopt;
		}
		if (terminals.size() >= std::numeric_limits<int>::digits) throw std::length_error("SteinerTree terminals");
		int states = int(1U << terminals.size());
		if (std::size_t(states) > std::vector<Sum>().max_size() / std::size_t(n_)) throw std::length_error("SteinerTree states");
		std::size_t count = std::size_t(states) * n_;
		std::vector<Sum> dp(count, inf);
		std::vector<int> trace;
		if constexpr (Restore) trace.assign(count, -1);
		bool dense = std::uint64_t(n_) * n_ <= 4 * std::uint64_t(edges_.size());
		std::vector<unsigned char> used(dense ? n_ : 0);
		for (int mask = 1; mask < states; ++mask) {
			std::size_t offset = std::size_t(mask) * n_;
			Sum *current = dp.data() + offset;
			if (std::has_single_bit(unsigned(mask))) current[terminals[std::countr_zero(unsigned(mask))]] = 0;
			else for (int part = (mask - 1) & mask; part > (mask ^ part); part = (part - 1) & mask) {
				const Sum *left = dp.data() + std::size_t(part) * n_;
				const Sum *right = dp.data() + std::size_t(mask ^ part) * n_;
				for (int v = 0; v < n_; ++v) {
					if (right[v] >= inf - left[v]) continue;
					Sum next = left[v] + right[v];
					if (next < current[v]) {
						current[v] = next;
						if constexpr (Restore) trace[offset + v] = -part - 1;
					}
				}
			}
			auto relax = [&](int v, Sum cost, auto push) {
				for (auto [u, id] : graph[v]) {
					Sum weight = Sum(edges_[id].cost);
					if (weight >= inf - cost) continue;
					Sum next = cost + weight;
					if (next < current[u]) {
						current[u] = next;
						if constexpr (Restore) trace[offset + u] = id;
						push(next, u);
					}
				}
			};
			if (dense) {
				std::fill(used.begin(), used.end(), 0);
				for (int iteration = 0; iteration < n_; ++iteration) {
					int v = -1;
					Sum cost = inf;
					for (int u = 0; u < n_; ++u) if (!used[u] && current[u] < cost) cost = current[u], v = u;
					if (v < 0 || (mask == states - 1 && cost >= current[root])) break;
					used[v] = 1;
					relax(v, cost, [](Sum, int) {});
				}
			} else {
				shortest_path_detail::DistanceQueue<Sum> queue;
				for (int v = 0; v < n_; ++v) if (current[v] < inf) queue.push(current[v], v);
				while (!queue.empty()) {
					auto [cost, v] = queue.pop();
					if (cost != current[v]) continue;
					if (mask == states - 1 && cost >= current[root]) break;
					relax(v, cost, [&](Sum next, int u) { queue.push(next, u); });
				}
			}
		}
		Sum answer = dp[std::size_t(states - 1) * n_ + root];
		if (!(answer < inf)) return std::nullopt;
		Result result;
		result.cost = answer;
		if constexpr (Restore) {
			UnionFind uf(n_);
			std::vector<std::pair<int, int>> work{ {states - 1, root} };
			while (!work.empty()) {
				auto [mask, v] = work.back();
				work.pop_back();
				while (true) {
					int step = trace[std::size_t(mask) * n_ + v];
					if (step == -1) break;
					if (step < -1) {
						int part = -step - 1;
						work.emplace_back(mask ^ part, v);
						mask = part;
					} else {
						const auto &e = edges_[step];
						if (!uf.same(e.from, e.to)) {
							uf.merge(e.from, e.to);
							result.edges.push_back(step);
						}
						v ^= e.from ^ e.to;
					}
				}
			}
		}
		return result;
	}

  public:
	explicit SteinerTree(int n = 0) : n_(n) { assert(0 <= n && n < std::numeric_limits<int>::max()); }
	int size() const { return n_; }
	int edge_count() const { return int(edges_.size()); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(std::numeric_limits<int>::max() / 2));
		edges_.reserve(m);
	}
	int add_edge(int u, int v, Cost cost) {
		assert(0 <= u && u < n_ && 0 <= v && v < n_ && cost >= 0);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		int id = edge_count();
		edges_.push_back({u, v, cost});
		return id;
	}
	Edge get_edge(int id) const { assert(0 <= id && id < edge_count()); return edges_[id]; }
	const std::vector<Edge> &edges() const { return edges_; }
	void clear() { edges_.clear(); }
	std::optional<Result> solve(std::vector<int> terminals, Sum inf = std::numeric_limits<Sum>::max()) const {
		return run<true>(std::move(terminals), inf);
	}
	std::optional<Sum> minimum_cost(std::vector<int> terminals, Sum inf = std::numeric_limits<Sum>::max()) const {
		auto result = run<false>(std::move(terminals), inf);
		if (!result) return std::nullopt;
		return result->cost;
	}
};

#endif
