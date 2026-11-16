#ifndef XOR_SET_HPP
#define XOR_SET_HPP

#include "../components/predecessor_set.hpp"
#include "../components/xor_select.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

template <int Bits = 30>
class XorSet {
	static_assert(1 <= Bits && Bits <= 31);
	static constexpr int leaf_bits = std::min(Bits, 9);
	static constexpr std::uint32_t leaf_mask = (std::uint32_t(1) << leaf_bits) - 1;
	static constexpr std::uint32_t block_count = std::uint32_t(1) << (Bits - leaf_bits);
	static constexpr std::uint32_t singleton = std::uint32_t(1) << 31;
	struct alignas(64) Leaf {
		std::array<std::uint64_t, 8> words{};
	};
	std::vector<std::uint32_t> directory;
	PredecessorSet occupied;
	std::vector<Leaf> leaves;
	std::uint32_t free_leaf = 0, all_xor = 0;
	std::int64_t count = 0;
	void ensure_storage() {
		if (!directory.empty()) return;
		std::vector<std::uint32_t> next(block_count);
		PredecessorSet index(block_count);
		directory.swap(next);
		occupied.swap(index);
	}
	std::uint32_t make_leaf() {
		if (free_leaf) {
			auto id = free_leaf;
			free_leaf = std::uint32_t(leaves[id].words[0]);
			leaves[id] = Leaf{};
			return id;
		}
		if (leaves.empty()) leaves.emplace_back();
		leaves.emplace_back();
		return std::uint32_t(leaves.size() - 1);
	}
	void release_leaf(std::uint32_t id) {
		leaves[id].words[0] = free_leaf;
		free_leaf = id;
	}
	static void check([[maybe_unused]] std::uint32_t value) { assert(value < (std::uint32_t(1) << Bits)); }
public:
	using value_type = std::uint32_t;
	XorSet() = default;
	XorSet(const XorSet &) = default;
	XorSet &operator=(const XorSet &other) {
		if (this != &other) { XorSet copy(other); swap(copy); }
		return *this;
	}
	XorSet(XorSet &&other) noexcept
		: directory(std::move(other.directory)), occupied(std::move(other.occupied)), leaves(std::move(other.leaves)),
		free_leaf(std::exchange(other.free_leaf, 0)), all_xor(std::exchange(other.all_xor, 0)), count(std::exchange(other.count, 0)) {
		other.directory.clear(); other.leaves.clear();
	}
	XorSet &operator=(XorSet &&other) noexcept {
		if (this != &other) { XorSet moved(std::move(other)); swap(moved); }
		return *this;
	}
	std::int64_t size() const { return count; }
	bool empty() const { return count == 0; }
	bool contains(std::uint32_t value) const {
		check(value);
		if (empty()) return false;
		auto entry = directory[value >> leaf_bits];
		if (!entry) return false;
		if (entry & singleton) return entry == (singleton | (value & leaf_mask));
		return leaves[entry].words[(value & leaf_mask) >> 6] >> (value & 63) & 1;
	}
	bool insert(std::uint32_t value) {
		check(value);
		ensure_storage();
		auto &entry = directory[value >> leaf_bits];
		auto suffix = value & leaf_mask;
		if (!entry) {
			occupied.insert(value >> leaf_bits);
			entry = singleton | suffix;
		} else if (entry & singleton) {
			auto old = entry ^ singleton;
			if (old == suffix) return false;
			auto id = make_leaf();
			leaves[id].words[old >> 6] = std::uint64_t(1) << (old & 63);
			leaves[id].words[suffix >> 6] |= std::uint64_t(1) << (suffix & 63);
			entry = id;
		} else {
			auto &word = leaves[entry].words[suffix >> 6];
			auto bit = std::uint64_t(1) << (suffix & 63);
			if (word & bit) return false;
			word |= bit;
		}
		++count; all_xor ^= value;
		return true;
	}
	bool erase(std::uint32_t value) {
		check(value);
		if (empty()) return false;
		auto &entry = directory[value >> leaf_bits];
		if (!entry) return false;
		if (entry & singleton) {
			if (entry != (singleton | (value & leaf_mask))) return false;
			entry = 0;
			occupied.erase(value >> leaf_bits);
		} else {
			auto &leaf = leaves[entry];
			auto &word = leaf.words[(value & leaf_mask) >> 6];
			auto bit = std::uint64_t(1) << (value & 63);
			if (!(word & bit)) return false;
			word ^= bit;
			std::uint64_t any = 0;
			for (auto part : leaf.words) any |= part;
			if (!any) {
				release_leaf(entry);
				entry = 0;
				occupied.erase(value >> leaf_bits);
			}
		}
		--count; all_xor ^= value;
		return true;
	}
	std::uint32_t argmin_xor(std::uint32_t value) const {
		assert(!empty());
		if (count == 1) return all_xor;
		auto prefix = (value >> leaf_bits) & (block_count - 1);
		if (!directory[prefix]) prefix = occupied.argmin_xor(value >> leaf_bits);
		auto entry = directory[prefix];
		if (entry & singleton) return (prefix << leaf_bits) | (entry ^ singleton);
		const auto &leaf = leaves[entry];
		auto preferred = (value & leaf_mask) >> 6;
		for (unsigned distance = 0; distance < 8; ++distance) {
			auto i = preferred ^ distance;
			if (leaf.words[i]) return (prefix << leaf_bits) | (i << 6) | xor_select_detail::argmin(leaf.words[i], value & 63);
		}
		std::unreachable();
	}
	std::uint32_t argmax_xor(std::uint32_t value) const { return argmin_xor(~value); }
	std::uint32_t min_xor(std::uint32_t value) const { return argmin_xor(value) ^ value; }
	std::uint32_t max_xor(std::uint32_t value) const { return argmax_xor(value) ^ value; }
	void clear() {
		std::fill(directory.begin(), directory.end(), 0);
		occupied.clear(); leaves.clear();
		free_leaf = all_xor = 0; count = 0;
	}
	void swap(XorSet &other) noexcept {
		directory.swap(other.directory); occupied.swap(other.occupied); leaves.swap(other.leaves);
		std::swap(free_leaf, other.free_leaf); std::swap(all_xor, other.all_xor); std::swap(count, other.count);
	}
	std::size_t memory_usage() const { return directory.capacity() * sizeof(std::uint32_t) + occupied.memory_usage() + leaves.capacity() * sizeof(Leaf); }
};

template <int Bits> requires (32 <= Bits && Bits <= 64)
class XorSet<Bits> {
	struct Node {
		std::uint64_t value = 0;
		int child[2]{-1, -1};
		int bit = -1;
	};
	std::vector<Node> nodes;
	int root = -1, free_node = -1;
	std::int64_t count = 0;
	static void check([[maybe_unused]] std::uint64_t value) {
		if constexpr (Bits < 64) assert((value >> Bits) == 0);
	}
	int make_node(Node node) {
		if (free_node >= 0) {
			int id = free_node;
			free_node = nodes[id].child[0];
			nodes[id] = node;
			return id;
		}
		if (nodes.size() >= std::size_t(std::numeric_limits<int>::max())) throw std::length_error("XorSet");
		int id = int(nodes.size());
		nodes.push_back(node);
		return id;
	}
	void release_node(int id) {
		nodes[id].child[0] = free_node;
		free_node = id;
	}
	int find_leaf(std::uint64_t value) const {
		int id = root;
		while (nodes[id].bit >= 0) id = nodes[id].child[(value >> nodes[id].bit) & 1];
		return id;
	}
public:
	using value_type = std::uint64_t;
	XorSet() = default;
	XorSet(const XorSet &) = default;
	XorSet &operator=(const XorSet &other) {
		if (this != &other) { XorSet copy(other); swap(copy); }
		return *this;
	}
	XorSet(XorSet &&other) noexcept
		: nodes(std::move(other.nodes)), root(std::exchange(other.root, -1)),
		free_node(std::exchange(other.free_node, -1)), count(std::exchange(other.count, 0)) { other.nodes.clear(); }
	XorSet &operator=(XorSet &&other) noexcept {
		if (this != &other) { XorSet moved(std::move(other)); swap(moved); }
		return *this;
	}
	std::int64_t size() const { return count; }
	bool empty() const { return count == 0; }
	bool contains(std::uint64_t value) const {
		check(value);
		return root >= 0 && nodes[find_leaf(value)].value == value;
	}
	bool insert(std::uint64_t value) {
		check(value);
		if (root < 0) {
			root = make_node(Node{value});
			++count;
			return true;
		}
		std::uint64_t difference = value ^ nodes[find_leaf(value)].value;
		if (!difference) return false;
		int bit = int(std::bit_width(difference)) - 1, parent = -1, side = 0, target = root;
		while (nodes[target].bit > bit) {
			parent = target;
			side = (value >> nodes[target].bit) & 1;
			target = nodes[target].child[side];
		}
		int leaf = make_node(Node{value}), branch;
		Node node;
		node.bit = bit;
		node.child[(value >> bit) & 1] = leaf;
		node.child[((value >> bit) & 1) ^ 1] = target;
		try { branch = make_node(node); }
		catch (...) { release_node(leaf); throw; }
		if (parent < 0) root = branch;
		else nodes[parent].child[side] = branch;
		++count;
		return true;
	}
	bool erase(std::uint64_t value) {
		check(value);
		if (root < 0) return false;
		int id = root, parent = -1, grandparent = -1, side = 0, parent_side = 0;
		while (nodes[id].bit >= 0) {
			grandparent = parent;
			parent_side = side;
			parent = id;
			side = (value >> nodes[id].bit) & 1;
			id = nodes[id].child[side];
		}
		if (nodes[id].value != value) return false;
		if (parent < 0) root = -1;
		else {
			int sibling = nodes[parent].child[side ^ 1];
			if (grandparent < 0) root = sibling;
			else nodes[grandparent].child[parent_side] = sibling;
			release_node(parent);
		}
		release_node(id);
		--count;
		return true;
	}
	std::uint64_t argmin_xor(std::uint64_t value) const {
		assert(!empty());
		return nodes[find_leaf(value)].value;
	}
	std::uint64_t argmax_xor(std::uint64_t value) const { return argmin_xor(~value); }
	std::uint64_t min_xor(std::uint64_t value) const { return argmin_xor(value) ^ value; }
	std::uint64_t max_xor(std::uint64_t value) const { return argmax_xor(value) ^ value; }
	void clear() { nodes.clear(); root = free_node = -1; count = 0; }
	void swap(XorSet &other) noexcept {
		nodes.swap(other.nodes);
		std::swap(root, other.root); std::swap(free_node, other.free_node); std::swap(count, other.count);
	}
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};

#endif
