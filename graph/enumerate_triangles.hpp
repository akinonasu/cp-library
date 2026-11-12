#ifndef ENUMERATE_TRIANGLES_HPP
#define ENUMERATE_TRIANGLES_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <span>
#include <utility>
#include <vector>

template<class F>
void enumerate_triangles(int n, std::span<const std::pair<int, int>> edges, F &&callback) {
	assert(n >= 0 && edges.size() <= std::size_t(std::numeric_limits<int>::max()));
	std::vector<int> degree(n), start(std::size_t(n) + 1);
	for (auto [u, v] : edges) {
		assert(0 <= u && u < n && 0 <= v && v < n && u != v);
		++degree[u], ++degree[v];
	}
	auto reverse = [&](int u, int v) { return degree[u] > degree[v] || (degree[u] == degree[v] && u > v); };
	for (auto [u, v] : edges) {
		if (reverse(u, v)) std::swap(u, v);
		++start[std::size_t(u) + 1];
	}
	for (int v = 0; v < n; ++v) start[v + 1] += start[v];
	auto cursor = start;
	std::vector<int> adjacent(edges.size());
	for (auto [u, v] : edges) {
		if (reverse(u, v)) std::swap(u, v);
		adjacent[cursor[u]++] = v;
	}
	std::fill(degree.begin(), degree.end(), -1);
	for (int u = 0; u < n; ++u) {
		if (start[u + 1] - start[u] < 2) continue;
		for (int i = start[u]; i < start[u + 1]; ++i) degree[adjacent[i]] = u;
		for (int i = start[u]; i < start[u + 1]; ++i) {
			int v = adjacent[i];
			for (int j = start[v]; j < start[v + 1]; ++j) {
				int w = adjacent[j];
				if (degree[w] != u) continue;
				int a = u, b = v, c = w;
				if (a > b) std::swap(a, b);
				if (b > c) std::swap(b, c);
				if (a > b) std::swap(a, b);
				callback(a, b, c);
			}
		}
	}
}

inline std::vector<std::array<int, 3>> enumerate_triangles(int n, std::span<const std::pair<int, int>> edges) {
	std::vector<std::array<int, 3>> result;
	enumerate_triangles(n, edges, [&](int u, int v, int w) { result.push_back({u, v, w}); });
	return result;
}

#endif
