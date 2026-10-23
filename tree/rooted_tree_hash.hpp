#ifndef ROOTED_TREE_HASH_HPP
#define ROOTED_TREE_HASH_HPP

#include "../components/tree_hash_core.hpp"
#include <bit>
#include <stdexcept>
#include <utility>

class RootedTreeHash {
	std::uint64_t seed_ = default_seed();

  public:
	using Hash = TreeHashValue;
	int root = -1;
	std::vector<Hash> hash;
	std::vector<int> parent, height, subtree_size;
	static std::uint64_t default_seed() { return tree_hash_detail::default_seed(); }
	RootedTreeHash() = default;
	explicit RootedTreeHash(const std::vector<std::vector<int>> &graph, int root = 0, std::uint64_t seed = default_seed()) {
		build(graph, root, seed);
	}
	void build(const std::vector<std::vector<int>> &graph, int root = 0, std::uint64_t seed = default_seed()) {
		auto tree = tree_hash_detail::root_tree(graph, root);
		hash = tree_hash_detail::hashes(graph, tree, seed);
		parent = std::move(tree.parent);
		height = std::move(tree.height);
		subtree_size = std::move(tree.size);
		this->root = graph.empty() ? -1 : root;
		seed_ = seed;
	}
	void build_parents(const std::vector<int> &parents, std::uint64_t seed = default_seed()) {
		auto tree = tree_hash_detail::root_parents(parents);
		hash = tree_hash_detail::hashes(tree, seed);
		root = tree.order.empty() ? -1 : tree.order[0];
		parent = std::move(tree.parent);
		height = std::move(tree.height);
		subtree_size = std::move(tree.size);
		seed_ = seed;
	}
	std::vector<int> classify() const {
		std::vector<int> result(size());
		if (empty()) return result;
		if (std::size_t(size()) > std::numeric_limits<std::size_t>::max() / 4) throw std::length_error("RootedTreeHash classify");
		std::size_t capacity = std::bit_ceil(std::size_t(size()) * 2);
		std::vector<int> table(capacity, -1);
		TreeHashValue::Hasher hasher;
		int count = 0;
		for (int v = 0; v < size(); ++v) {
			std::size_t slot = hasher(hash[v]) & (capacity - 1);
			while (table[slot] != -1 && hash[table[slot]] != hash[v]) slot = (slot + 1) & (capacity - 1);
			if (table[slot] == -1) table[slot] = v, result[v] = count++;
			else result[v] = result[table[slot]];
		}
		return result;
	}
	int size() const { return static_cast<int>(hash.size()); }
	bool empty() const { return hash.empty(); }
	std::uint64_t seed() const { return seed_; }
	Hash get() const { return empty() ? Hash{} : hash[root]; }
	Hash get(int v) const {
		assert(0 <= v && v < size());
		return hash[v];
	}
	Hash operator[](int v) const { return get(v); }
};

#endif
