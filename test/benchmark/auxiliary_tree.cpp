#include "graph/auxiliary_tree.hpp"
#include "graph/lca.hpp"
#include "graph/hld.hpp"
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

template<class Tree> class Baseline {
	const Tree *tree;
	std::vector<std::pair<int, int>> keys;
	std::vector<int> stack;
	std::vector<int> chosen;
	using Node = typename AuxiliaryTree<Tree>::Node;
	std::vector<Node> nodes;
  public:
	explicit Baseline(const Tree &t) : tree(&t) {}
	const std::vector<Node> &build(std::span<const int> input, bool sorted) {
		keys.clear();
		for (int v : input) keys.emplace_back(tree->subtree(v).first, v);
		if (!sorted) std::sort(keys.begin(), keys.end());
		keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
		std::size_t k = keys.size();
		chosen.clear();
		for (auto [position, v] : keys) {
			(void)v;
			chosen.push_back(position);
		}
		for (std::size_t i = 1; i < k; ++i) {
			int w = tree->lca(keys[i - 1].second, keys[i].second);
			keys.emplace_back(tree->subtree(w).first, w);
		}
		std::sort(keys.begin(), keys.end());
		keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
		nodes.clear();
		stack.clear();
		std::size_t mark = 0;
		for (auto [position, v] : keys) {
			while (!stack.empty() && !tree->is_ancestor(nodes[stack.back()].vertex, v)) stack.pop_back();
			int p = stack.empty() ? -1 : stack.back();
			int length = p < 0 ? 0 : tree->depth[v] - tree->depth[nodes[p].vertex];
			bool selected = mark < chosen.size() && chosen[mark] == position;
			mark += selected;
			nodes.push_back({v, p, length, selected});
			stack.push_back(static_cast<int>(nodes.size()) - 1);
		}
		return nodes;
	}
	const std::vector<Node> &build(std::span<const int> input) { return build(input, false); }
	const std::vector<Node> &build_sorted(std::span<const int> input) { return build(input, true); }
};

template<class Node> std::uint64_t checksum(const std::vector<Node> &nodes) {
	std::uint64_t result = 0;
	for (const auto &node : nodes) {
		std::uint64_t value = static_cast<unsigned>(node.vertex) + 1;
		value = value * 1000003 + (node.parent < 0 ? 0 : nodes[node.parent].vertex + 1);
		value = value * 1000033 + node.length;
		value = value * 1000037 + node.selected;
		value ^= value >> 30;
		value *= 0xbf58476d1ce4e5b9ULL;
		result += value ^ (value >> 27);
	}
	return result;
}

template<class Builder>
void run(Builder &builder, const std::vector<std::vector<int>> &queries, bool sorted) {
	std::uint64_t sum = 0;
	auto start = std::chrono::steady_clock::now();
	for (const auto &keys : queries) sum = sum * 1000003 + checksum(sorted ? builder.build_sorted(keys) : builder.build(keys));
	double elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
	std::cout << std::fixed << std::setprecision(6) << elapsed << ' ' << sum << '\n';
}

template<class Tree>
void solve(const std::string &shape, const std::string &method, int n, int k, int q) {
	std::mt19937 rng(612409);
	std::vector<int> labels(n);
	std::iota(labels.begin(), labels.end(), 0);
	std::shuffle(labels.begin(), labels.end(), rng);
	std::vector<std::pair<int, int>> edges;
	for (int v = 1; v < n; ++v) {
		int p = shape == "path" ? v - 1 : shape == "star" ? 0 : shape == "balanced" ? (v - 1) / 2 : rng() % v;
		edges.emplace_back(labels[p], labels[v]);
	}
	std::shuffle(edges.begin(), edges.end(), rng);
	Tree tree(n);
	for (auto [u, v] : edges) tree.add_edge(u, v);
	tree.build(labels[0]);
	std::vector<std::vector<int>> queries(q, std::vector<int>(k));
	bool sorted = method.ends_with("sorted");
	for (auto &keys : queries) {
		for (int &v : keys) v = rng() % n;
		if (sorted) std::sort(keys.begin(), keys.end(), [&](int u, int v) { return tree.subtree(u).first < tree.subtree(v).first; });
	}
	if (method.starts_with("baseline")) {
		Baseline builder(tree);
		run(builder, queries, sorted);
	} else {
		AuxiliaryTree builder(tree);
		run(builder, queries, sorted);
	}
}

int main(int argc, char **argv) {
	if (argc != 7) return 1;
	std::string shape = argv[1], method = argv[2];
	int n = std::stoi(argv[3]), k = std::stoi(argv[4]), q = std::stoi(argv[5]);
	if (std::string(argv[6]) == "hld") solve<HLD>(shape, method, n, k, q);
	else solve<LCA>(shape, method, n, k, q);
}
