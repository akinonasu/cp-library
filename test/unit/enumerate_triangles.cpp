#include "graph/enumerate_triangles.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using Edges = std::vector<std::pair<int, int>>;
using Triangle = std::array<int, 3>;

std::vector<Triangle> reference(int n, const Edges &edges) {
	std::vector<std::vector<bool>> adjacent(n, std::vector<bool>(n));
	for (auto [u, v] : edges) adjacent[u][v] = adjacent[v][u] = true;
	std::vector<Triangle> result;
	for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v)
		for (int w = v + 1; w < n; ++w)
			if (adjacent[u][v] && adjacent[u][w] && adjacent[v][w]) result.push_back({u, v, w});
	return result;
}

void check(int n, const Edges &edges) {
	auto original = edges;
	auto expected = reference(n, edges);
	auto result = enumerate_triangles(n, edges);
	std::sort(result.begin(), result.end());
	assert(result == expected);
	std::vector<Triangle> collected;
	enumerate_triangles(n, edges, [&](int u, int v, int w) {
		assert(0 <= u && u < v && v < w && w < n);
		collected.push_back({u, v, w});
		return false;
	});
	std::sort(collected.begin(), collected.end());
	assert(collected == expected && edges == original);
}

void large_shapes() {
	constexpr int n = 250000;
	Edges edges;
	edges.reserve(2 * n);
	for (int v = 1; v < n; ++v) edges.emplace_back(v, 0);
	assert(enumerate_triangles(n, edges).empty());
	edges.clear();
	for (int v = 1; v < n; ++v) edges.emplace_back(v, v - 1);
	assert(enumerate_triangles(n, edges).empty());
	edges.emplace_back(n - 1, 0);
	assert(enumerate_triangles(n, edges).empty());
	edges.clear();
	for (int v = 1; v < n - 1; ++v) {
		edges.emplace_back(v, 0);
		edges.emplace_back(n - 1, v);
	}
	edges.emplace_back(n - 1, 0);
	std::vector<unsigned char> seen(n);
	enumerate_triangles(n, edges, [&](int u, int v, int w) {
		assert(u == 0 && w == n - 1 && 0 < v && v < n - 1 && !seen[v]);
		seen[v] = true;
	});
	for (int v = 1; v < n - 1; ++v) assert(seen[v]);
	edges.clear();
	constexpr int side = 400;
	for (int u = 0; u < side; ++u) for (int v = 0; v < side; ++v)
		edges.emplace_back(u, side + v);
	assert(enumerate_triangles(n, edges).empty());
	edges.clear();
	constexpr int clique = 180;
	for (int u = 0; u < clique; ++u) for (int v = u + 1; v < clique; ++v)
		edges.emplace_back(3 * u + 1, 3 * v + 1);
	std::uint64_t count = 0, vertex_sum = 0;
	enumerate_triangles(n, edges, [&](int u, int v, int w) {
		assert(0 < u && u < v && v < w && w < 3 * clique);
		assert(u % 3 == 1 && v % 3 == 1 && w % 3 == 1);
		++count;
		vertex_sum += u + v + w;
	});
	assert(count == std::uint64_t(clique) * (clique - 1) * (clique - 2) / 6);
	std::uint64_t label_sum = std::uint64_t(clique) * (3 * clique - 1) / 2;
	assert(vertex_sum == label_sum * (clique - 1) * (clique - 2) / 2);
	assert(enumerate_triangles(1000000, Edges{}).empty());
}

int main() {
	for (int n = 0; n <= 6; ++n) {
		Edges possible;
		for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) possible.emplace_back(u, v);
		for (unsigned mask = 0; mask < (1U << possible.size()); ++mask) {
			Edges edges;
			for (std::size_t i = 0; i < possible.size(); ++i)
				if (mask >> i & 1U) edges.push_back(possible[i]);
			check(n, edges);
		}
	}
	std::mt19937 random(841094);
	for (int trial = 0; trial < 1000; ++trial) {
		int n = random() % 45;
		Edges edges;
		unsigned density = trial % 11;
		for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v)
			if (random() % 10 < density) edges.emplace_back(u, v);
		check(n, edges);
		std::vector<int> label(n);
		std::iota(label.begin(), label.end(), 0);
		std::shuffle(label.begin(), label.end(), random);
		for (auto &[u, v] : edges) {
			u = label[u]; v = label[v];
			if (random() & 1U) std::swap(u, v);
		}
		std::shuffle(edges.begin(), edges.end(), random);
		check(n, edges);
	}
	const std::array<std::pair<int, int>, 3> edges{ std::pair{2, 1}, {0, 2}, {1, 0} };
	assert((enumerate_triangles(3, edges) == std::vector<Triangle>{Triangle{0, 1, 2}}));
	std::vector<Triangle> seen;
	auto callback = [state = std::make_unique<int>(0), &seen](int u, int v, int w) mutable {
		++*state;
		seen.push_back({u, v, w});
	};
	enumerate_triangles(3, edges, callback);
	enumerate_triangles(3, edges, std::move(callback));
	assert((seen == std::vector<Triangle>{Triangle{0, 1, 2}, Triangle{0, 1, 2}}));
	bool propagated = false;
	try {
		enumerate_triangles(3, edges, [](int, int, int) { throw std::runtime_error("callback"); });
	} catch (const std::runtime_error &) { propagated = true; }
	assert(propagated);
	large_shapes();
}
