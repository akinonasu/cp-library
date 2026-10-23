#ifndef TREE_HASH_CORE_HPP
#define TREE_HASH_CORE_HPP

#include "splitmix64.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

struct TreeHashValue {
	std::array<std::uint64_t, 2> value{};
	auto operator<=>(const TreeHashValue &) const = default;
	struct Hasher {
		std::size_t operator()(const TreeHashValue &hash) const noexcept {
			return splitmix64(hash.value[0] ^ splitmix64(hash.value[1]));
		}
	};
};

namespace tree_hash_detail {

inline constexpr std::uint64_t mod = (std::uint64_t(1) << 61) - 1;

inline std::uint64_t default_seed() {
	static const std::uint64_t seed = splitmix64(std::uint64_t(std::chrono::steady_clock::now().time_since_epoch().count()) ^
		std::uint64_t(reinterpret_cast<std::uintptr_t>(&mod)));
	return seed;
}

inline std::uint64_t mul(std::uint64_t a, std::uint64_t b) {
	__uint128_t product = __uint128_t(a) * b;
	std::uint64_t value = (std::uint64_t(product) & mod) + std::uint64_t(product >> 61);
	return value >= mod ? value - mod : value;
}

struct Rooting {
	std::vector<int> parent, order, height, size;
};

inline Rooting root_tree(const std::vector<std::vector<int>> &graph, int root) {
	assert(graph.size() <= std::size_t(std::numeric_limits<int>::max()));
	const int n = static_cast<int>(graph.size());
	assert(n ? 0 <= root && root < n : root == 0 || root == -1);
	Rooting result;
	if (!n) return result;
	result.parent.assign(n, -2);
	result.height.assign(n, 0);
	result.size.assign(n, 1);
	result.order.reserve(n);
	result.parent[root] = -1;
	result.order.push_back(root);
	[[maybe_unused]] std::size_t edges = 0;
	for (std::size_t i = 0; i < result.order.size(); ++i) {
		int v = result.order[i];
		edges += graph[v].size();
		for (int u : graph[v]) {
			assert(0 <= u && u < n && u != v);
			if (u == result.parent[v]) continue;
			assert(result.parent[u] == -2);
			result.parent[u] = v;
			result.order.push_back(u);
		}
	}
	assert(result.order.size() == graph.size() && edges == std::size_t(n - 1) * 2);
	for (auto it = result.order.rbegin(); it != result.order.rend(); ++it) {
		int v = *it, p = result.parent[v];
		if (p == -1) continue;
		result.height[p] = std::max(result.height[p], result.height[v] + 1);
		result.size[p] += result.size[v];
	}
	return result;
}

inline Rooting root_parents(const std::vector<int> &parent) {
	assert(parent.size() <= std::size_t(std::numeric_limits<int>::max()));
	const int n = static_cast<int>(parent.size());
	Rooting result;
	result.parent = parent;
	result.height.assign(n, 0);
	result.size.assign(n, 1);
	result.order.reserve(n);
	std::vector<int> degree(n);
	int root = -1;
	for (int v = 0; v < n; ++v) {
		int p = parent[v];
		assert(p == -1 || (0 <= p && p < n && p != v));
		if (p == -1) { assert(root == -1); root = v; }
		else ++degree[p];
	}
	assert(!n || root != -1);
	for (int v = 0; v < n; ++v) if (!degree[v]) result.order.push_back(v);
	for (std::size_t i = 0; i < result.order.size(); ++i) {
		int v = result.order[i], p = parent[v];
		if (p == -1) continue;
		result.height[p] = std::max(result.height[p], result.height[v] + 1);
		result.size[p] += result.size[v];
		if (!--degree[p]) result.order.push_back(p);
	}
	assert(result.order.size() == parent.size());
	std::reverse(result.order.begin(), result.order.end());
	return result;
}

inline TreeHashValue salt_at(int height, std::uint64_t seed) {
	SplitMix64 random(seed + std::uint64_t(height) * 2 * 0x9e3779b97f4a7c15ULL);
	return { {random() % (mod - 1) + 1, random() % (mod - 1) + 1} };
}

inline void append(TreeHashValue &hash, const TreeHashValue &child, const TreeHashValue &salt) {
	for (int k = 0; k < 2; ++k) {
		std::uint64_t factor = salt.value[k] + child.value[k];
		if (factor >= mod) factor -= mod;
		hash.value[k] = mul(hash.value[k], factor);
	}
}

inline std::vector<TreeHashValue> hashes(const Rooting &tree, std::uint64_t seed) {
	std::vector<TreeHashValue> result(tree.parent.size(), TreeHashValue{ {1, 1} });
	if (result.empty()) return result;
	std::vector<TreeHashValue> salt(tree.height[tree.order[0]] + 1);
	SplitMix64 random(seed);
	for (auto &value : salt) value = { {random() % (mod - 1) + 1, random() % (mod - 1) + 1} };
	for (auto it = tree.order.rbegin(); it + 1 != tree.order.rend(); ++it) {
		int v = *it, p = tree.parent[v];
		append(result[p], result[v], salt[tree.height[p]]);
	}
	return result;
}

inline std::vector<TreeHashValue> hashes(const std::vector<std::vector<int>> &graph, const Rooting &tree, std::uint64_t seed) {
	assert(graph.size() == tree.parent.size());
	return hashes(tree, seed);
}

}

#endif
