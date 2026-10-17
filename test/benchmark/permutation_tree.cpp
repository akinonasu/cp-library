#include "data_structure/permutation_tree.hpp"
#include "baseline.hpp"
#include "packed.hpp"
#include "aggregate.hpp"
#include "reference.hpp"
#include <bit>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

std::uint64_t mix(std::uint64_t x) {
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
	return x ^ (x >> 31);
}

std::uint64_t entry(int l, int r, int pl, int pr, int type) {
	return mix((std::uint64_t(l) << 32) | unsigned(r)) ^
		std::rotl(mix((std::uint64_t(pl + 1) << 32) | unsigned(pr + 1)), 17) ^ mix(type);
}

template <class Tree>
std::pair<std::size_t, std::uint64_t> calculate(const std::vector<int> &p) {
	Tree tree(p);
	std::uint64_t checksum = 0;
	for (auto &node : tree.nodes()) {
		int l = -1, r = -1;
		if (node.parent >= 0) l = tree[node.parent].l, r = tree[node.parent].r;
		checksum += entry(node.l, node.r, l, r, int(node.type));
	}
	return {tree.nodes().size(), checksum};
}

std::pair<std::size_t, std::uint64_t> reference(const std::vector<int> &p) {
	using Tree = nachia::CommonIntervalDecompositionTree;
	Tree tree(p);
	std::uint64_t checksum = 0;
	for (auto &node : tree.tree) {
		int l = -1, r = -1;
		if (node.parent >= 0) l = tree.tree[node.parent].l, r = tree.tree[node.parent].r;
		int type = node.type == Tree::One ? 0 : node.type == Tree::Inc ? 1 : node.type == Tree::Dec ? 2 : 3;
		checksum += entry(node.l, node.r, l, r, type);
	}
	return {tree.tree.size(), checksum};
}

int main(int argc, char **argv) {
	if (argc != 5) return 1;
	std::string variant = argv[1], kind = argv[2];
	int n = std::stoi(argv[3]), repetitions = std::stoi(argv[4]);
	std::vector<int> p(n);
	std::iota(p.begin(), p.end(), 0);
	std::mt19937 rng(279529);
	if (kind == "random") std::shuffle(p.begin(), p.end(), rng);
	else if (kind == "descending") std::reverse(p.begin(), p.end());
	else if (kind == "pairs") for (int i = 0; i + 1 < n; i += 2) std::swap(p[i], p[i + 1]);
	else if (kind == "zigzag") for (int i = 0; i < n; ++i) p[i] = i % 2 ? n - 1 - i / 2 : i / 2;
	else if (kind == "blocks") for (int i = 0; i < n; i += 256) std::shuffle(p.begin() + i, p.begin() + std::min(n, i + 256), rng);
	else if (kind == "recursive") {
		for (int length = 2; length <= n; length *= 2)
			for (int i = 0; i + length <= n; i += length)
				if (rng() & 1) std::rotate(p.begin() + i, p.begin() + i + length / 2, p.begin() + i + length);
	}
	std::size_t count = 0;
	std::uint64_t checksum = 0;
	auto begin = std::chrono::steady_clock::now();
	for (int i = 0; i < repetitions; ++i) {
		auto result = variant == "current" ? calculate<PermutationTree>(p) :
			variant == "baseline" ? calculate<PermutationTreeBaseline>(p) :
			variant == "aggregate" ? calculate<PermutationTreeAggregate>(p) :
			variant == "packed" ? calculate<PermutationTreePacked>(p) : reference(p);
		count = result.first;
		checksum = checksum * 1000000007 + result.second;
	}
	double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count() / repetitions;
	std::cout << "{\"nodes\":" << count << ",\"checksum\":\"" << checksum << "\",\"ms\":" << ms << "}\n";
}
