#include "tree/rooted_tree_hash.hpp"
#include "tree/tree_hash.hpp"
#include <functional>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <unordered_map>

using Graph = std::vector<std::vector<int>>;
using Hash = TreeHashValue;
using UnrootedHash = std::array<Hash, 2>;

std::string canonical(const Graph &graph, int v, int parent = -1) {
	std::vector<std::string> children;
	for (int u : graph[v]) if (u != parent) children.push_back(canonical(graph, u, v));
	std::sort(children.begin(), children.end());
	std::string result = "(";
	for (const auto &child : children) result += child;
	return result + ")";
}

std::string canonical(const Graph &graph) {
	std::string result;
	for (int v = 0; v < static_cast<int>(graph.size()); ++v) {
		auto candidate = canonical(graph, v);
		if (result.empty() || candidate < result) result = candidate;
	}
	return result;
}

Graph decode(const std::string &shape) {
	Graph graph;
	std::vector<int> path;
	for (char ch : shape) {
		if (ch == ')') path.pop_back();
		else {
			int v = static_cast<int>(graph.size());
			graph.emplace_back();
			if (!path.empty()) {
				int p = path.back();
				graph[p].push_back(v);
				graph[v].push_back(p);
			}
			path.push_back(v);
		}
	}
	return graph;
}

Graph permute(const Graph &graph, const std::vector<int> &label, std::mt19937_64 &random) {
	Graph result(graph.size());
	for (int v = 0; v < static_cast<int>(graph.size()); ++v)
		for (int u : graph[v]) result[label[v]].push_back(label[u]);
	for (auto &row : result) std::shuffle(row.begin(), row.end(), random);
	return result;
}

struct Registry {
	std::map<std::string, Hash> rooted;
	std::map<Hash, std::string> rooted_inverse;
	std::map<std::string, UnrootedHash> unrooted;
	std::map<UnrootedHash, std::string> unrooted_inverse;

	void check(const Graph &graph, int root, std::uint64_t seed) {
		auto saved = graph;
		RootedTreeHash tree(graph, root, seed);
		assert(tree.size() == static_cast<int>(graph.size()));
		assert(tree.root == root && tree.seed() == seed && tree.get() == tree[root]);
		std::function<std::pair<int, int>(int, int)> visit = [&](int v, int p) {
			assert(tree.parent[v] == p);
			int height = 0, size = 1;
			for (int u : graph[v]) if (u != p) {
				auto [h, s] = visit(u, v);
				height = std::max(height, h + 1);
				size += s;
			}
			assert(tree.height[v] == height && tree.subtree_size[v] == size);
			auto shape = canonical(graph, v, p);
			assert(rooted.try_emplace(shape, tree[v]).first->second == tree.get(v));
			assert(rooted_inverse.try_emplace(tree[v], shape).first->second == shape);
			for (auto x : tree[v].value) assert(x < tree_hash_detail::mod);
			return std::pair{height, size};
		};
		visit(root, -1);
		auto value = tree_hash(graph, seed);
		auto shape = canonical(graph);
		assert(unrooted.try_emplace(shape, value).first->second == value);
		assert(unrooted_inverse.try_emplace(value, shape).first->second == shape);
		std::vector<Hash> expected;
		for (int v = 0; v < tree.size(); ++v) {
			int largest = 0;
			for (int u : graph[v]) {
				std::function<int(int, int)> count = [&](int x, int p) {
					int result = 1;
					for (int y : graph[x]) if (y != p) result += count(y, x);
					return result;
				};
				largest = std::max(largest, count(u, v));
			}
			if (largest <= tree.size() / 2) expected.push_back(RootedTreeHash(graph, v, seed).get());
		}
		assert(1 <= expected.size() && expected.size() <= 2);
		if (expected.size() == 1) expected.push_back(expected[0]);
		std::sort(expected.begin(), expected.end());
		assert(expected[0] == value[0] && expected[1] == value[1]);
		assert(graph == saved);
	}
};

int main() {
	std::mt19937_64 random(20261022);
	const auto mod = tree_hash_detail::mod;
	for (auto a : {std::uint64_t(0), std::uint64_t(1), mod / 2, mod - 2, mod - 1})
		for (auto b : {std::uint64_t(0), std::uint64_t(1), mod / 2, mod - 2, mod - 1})
			assert(tree_hash_detail::mul(a, b) == __uint128_t(a) * b % mod);
	for (int i = 0; i < 100000; ++i) {
		auto a = random() % mod, b = random() % mod;
		assert(tree_hash_detail::mul(a, b) == __uint128_t(a) * b % mod);
	}
	RootedTreeHash empty;
	assert(empty.empty() && empty.root == -1 && empty.get() == Hash{});
	empty.build({}, -1, 0);
	assert(empty.empty() && empty.root == -1 && empty.get() == Hash{} && empty.seed() == 0);
	assert(tree_hash({}) == UnrootedHash{});
	std::vector<std::vector<std::string>> shapes(11);
	const std::array<int, 11> counts{0, 1, 1, 2, 4, 9, 20, 48, 115, 286, 719};
	for (int n = 1; n <= 10; ++n) {
		std::vector<std::pair<std::string, int>> available;
		for (int size = 1; size < n; ++size)
			for (const auto &shape : shapes[size]) available.emplace_back(shape, size);
		std::sort(available.begin(), available.end());
		std::function<void(int, int, std::string)> enumerate = [&](int start, int remaining, std::string children) {
			if (!remaining) {
				shapes[n].push_back("(" + children + ")");
				return;
			}
			for (int i = start; i < static_cast<int>(available.size()); ++i)
				if (available[i].second <= remaining) enumerate(i, remaining - available[i].second, children + available[i].first);
		};
		enumerate(0, n - 1, "");
		assert(static_cast<int>(shapes[n].size()) == counts[n]);
	}
	for (std::uint64_t seed : {std::uint64_t(0), std::uint64_t(42), std::numeric_limits<std::uint64_t>::max()}) {
		Registry registry;
		for (int n = 1; n <= 10; ++n) for (const auto &shape : shapes[n]) {
			auto graph = decode(shape);
			registry.check(graph, 0, seed);
			std::vector<int> labels(n);
			std::iota(labels.begin(), labels.end(), 0);
			std::shuffle(labels.begin(), labels.end(), random);
			auto renamed = permute(graph, labels, random);
			registry.check(renamed, labels[0], seed);
			assert(RootedTreeHash(graph, 0, seed).get() == RootedTreeHash(renamed, labels[0], seed).get());
			assert(tree_hash(graph, seed) == tree_hash(renamed, seed));
		}
		assert(registry.rooted.size() == 1205);
		for (int trial = 0; trial < 300; ++trial) {
			int n = 1 + random() % 50;
			Graph graph(n);
			for (int v = 1; v < n; ++v) {
				int p = random() % v;
				graph[p].push_back(v);
				graph[v].push_back(p);
			}
			registry.check(graph, random() % n, seed);
		}
	}
	Graph example{ {1}, {0, 2, 3}, {1}, {1} };
	RootedTreeHash first(example, 1), second(example, 0);
	assert(first.seed() == second.seed());
	assert(first[0] == first[2] && first[2] == first[3]);
	assert(first.get() != second.get());
	assert(tree_hash(example)[0] == first.get());
	std::unordered_map<Hash, int, Hash::Hasher> ids;
	ids[first[0]] = 7;
	assert(ids[first[2]] == 7);
	auto copy = first;
	assert(copy.hash == first.hash && copy.parent == first.parent && copy.height == first.height && copy.subtree_size == first.subtree_size);
	auto moved = std::move(copy);
	assert(moved.get() == first.get());
	copy.build(example, 0);
	assert(copy.hash == second.hash);
	first.build({}, 0);
	assert(first.empty() && first.root == -1 && first.parent.empty() && first.height.empty() && first.subtree_size.empty());
	for (bool star : {false, true}) {
		const int n = 500000;
		Graph graph(n);
		for (int v = 1; v < n; ++v) {
			int p = star ? 0 : v - 1;
			graph[p].push_back(v);
			graph[v].push_back(p);
		}
		RootedTreeHash tree(graph);
		assert(tree.subtree_size[0] == n && tree.height[0] == (star ? 1 : n - 1));
		auto value = tree_hash(graph);
		assert(value[0] == value[1]);
		if (star) assert(value[0] == tree.get());
		else {
			RootedTreeHash reverse(graph, n - 1);
			for (int i = 0; i < n; ++i) assert(tree[i] == reverse[n - 1 - i]);
			assert(value[0] == RootedTreeHash(graph, n / 2).get());
		}
	}
}
