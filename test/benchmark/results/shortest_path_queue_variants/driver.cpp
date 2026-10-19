#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <queue>
#include <string>
#include <utility>
#include <vector>
#include "components/fast_io.hpp"

#ifndef SP_BENCH_HEADER
#define SP_BENCH_HEADER "graph/dijkstra.hpp"
#endif
#include SP_BENCH_HEADER

using Weight = std::int64_t;
using Graph = std::vector<std::vector<std::pair<int, Weight>>>;
using Clock = std::chrono::steady_clock;

struct Edge { int from, to; Weight cost; };

double elapsed(Clock::time_point first, Clock::time_point last) {
	return std::chrono::duration<double, std::milli>(last - first).count();
}

std::vector<Weight> solve(const Graph &g, int source) {
#ifdef SP_REFERENCE_HEAP
	using State = std::pair<Weight, int>;
	std::priority_queue<State, std::vector<State>, std::greater<State>> heap;
	std::vector<Weight> dist(g.size(), DIJKSTRA_INF);
	dist[source] = 0;
	heap.emplace(0, source);
	while (!heap.empty()) {
		auto [d, v] = heap.top();
		heap.pop();
		if (d != dist[v]) continue;
		for (auto [to, cost] : g[v]) {
			if (cost >= DIJKSTRA_INF - d) continue;
			if (d + cost < dist[to]) {
				dist[to] = d + cost;
				heap.emplace(dist[to], to);
			}
		}
	}
	for (auto &d : dist) if (d == DIJKSTRA_INF) d = -1;
	return dist;
#else
#ifdef SP_TOPOLOGICAL
	return topological_shortest_path(g, source);
#else
	return dijkstra(g, source);
#endif
#endif
}

int main(int argc, char **argv) {
	bool whole = argc > 1 && std::string(argv[1]) == "whole";
	auto start = Clock::now();
	FastInput input;
	int n, m, source, target;
	input.read(n); input.read(m); input.read(source); input.read(target);
	std::vector<Edge> edges(m);
	for (auto &edge : edges) {
		input.read(edge.from); input.read(edge.to); input.read(edge.cost);
	}
	auto parsed = Clock::now();
	Graph g(n);
	for (auto edge : edges) g[edge.from].emplace_back(edge.to, edge.cost);
	auto built = Clock::now();
	std::vector<std::pair<int, int>> path;
#ifdef SP_TARGET_API
#ifdef SP_TOPOLOGICAL
	auto result = topological_shortest_path_to(g, source, target);
#else
	auto result = dijkstra_path(g, source, target);
#endif
	Weight distance = result.first;
	auto solved = Clock::now();
	for (std::size_t i = 1; i < result.second.size(); ++i) path.emplace_back(result.second[i-1], result.second[i]);
#else
	auto dist = solve(g, source);
	Weight distance = dist[target];
	auto solved = Clock::now();
	if (distance != -1) {
		std::vector<int> parent(n, -1), todo{source};
		parent[source] = source;
		for (std::size_t i = 0; i < todo.size(); ++i) {
			int v = todo[i];
			for (auto [to, cost] : g[v]) {
				if (parent[to] == -1 && dist[to] == dist[v] + cost) {
					parent[to] = v;
					todo.push_back(to);
				}
			}
		}
		for (int v = target; v != source; v = parent[v]) {
			if (parent[v] < 0) std::abort();
			path.emplace_back(parent[v], v);
		}
		std::reverse(path.begin(), path.end());
	}
#endif
	auto recovered = Clock::now();
	if (whole) {
		FastOutput output;
		if (distance == -1) output.println(-1);
		else {
			output.write(distance); output.write(' '); output.println(path.size());
			for (auto [u, v] : path) {
				output.write(u); output.write(' '); output.println(v);
			}
		}
		output.flush();
	}
	auto written = Clock::now();
	if (!whole && distance >= 0) {
		Weight total = 0;
		int vertex = source;
		for (auto [from, to] : path) {
			if (from != vertex || from < 0 || from >= n || to < 0 || to >= n) std::abort();
			Weight minimum = DIJKSTRA_INF;
			for (auto [v, cost] : g[from]) if (v == to) minimum = std::min(minimum, cost);
			if (minimum == DIJKSTRA_INF) std::abort();
			total += minimum;
			vertex = to;
		}
		if (vertex != target || total != distance) std::abort();
	}
#ifdef SP_TARGET_API
	std::uint64_t checksum = std::uint64_t(distance);
	std::size_t reached = 0;
#else
	std::uint64_t checksum = dist.size();
	std::size_t reached = 0;
	for (auto d : dist) {
		checksum = (checksum ^ std::uint64_t(d)) * 0x9e3779b97f4a7c15ULL;
		reached += d != -1;
	}
#endif
	std::fprintf(whole ? stderr : stdout,
		"{\"parse_ms\":%.9f,\"build_ms\":%.9f,\"solve_ms\":%.9f,\"recover_ms\":%.9f,\"output_ms\":%.9f,\"checksum\":\"%llu\",\"distance\":%lld,\"reached\":%zu,\"path_edges\":%zu}\n",
		elapsed(start, parsed), elapsed(parsed, built), elapsed(built, solved), elapsed(solved, recovered), elapsed(recovered, written),
		static_cast<unsigned long long>(checksum), static_cast<long long>(distance), reached, path.size());
}
