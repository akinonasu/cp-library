#ifndef ORDERED_MULTISET_BTREE_HPP
#define ORDERED_MULTISET_BTREE_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <type_traits>
#include <vector>
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif
#include "radix_sort.hpp"

namespace ordered_multiset_detail {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
template<bool Strict, class Key>
__attribute__((target("avx2"))) int find_avx2(const Key *data, int n, Key value) {
	unsigned less = 0;
	constexpr int lanes = 32 / sizeof(Key);
	for (int i = 0; i < n; i += lanes) {
		auto keys = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(data + i));
		__m256i limit, match;
		if constexpr (sizeof(Key) == 4) {
			limit = _mm256_set1_epi32(static_cast<int>(value));
			if constexpr (std::is_unsigned_v<Key>) { auto sign = _mm256_set1_epi32(-2147483647 - 1); keys = _mm256_xor_si256(keys, sign); limit = _mm256_xor_si256(limit, sign); }
			match = _mm256_cmpgt_epi32(limit, keys);
			if constexpr (Strict) match = _mm256_or_si256(match, _mm256_cmpeq_epi32(limit, keys));
			less |= static_cast<unsigned>(_mm256_movemask_ps(_mm256_castsi256_ps(match))) << i;
		} else {
			limit = _mm256_set1_epi64x(static_cast<long long>(value));
			if constexpr (std::is_unsigned_v<Key>) { auto sign = _mm256_set1_epi64x(-9223372036854775807LL - 1); keys = _mm256_xor_si256(keys, sign); limit = _mm256_xor_si256(limit, sign); }
			match = _mm256_cmpgt_epi64(limit, keys);
			if constexpr (Strict) match = _mm256_or_si256(match, _mm256_cmpeq_epi64(limit, keys));
			less |= static_cast<unsigned>(_mm256_movemask_pd(_mm256_castsi256_pd(match))) << i;
		}
	}
	if (n < 32) less &= (1U << n) - 1;
	return std::countr_one(less);
}
template<bool Strict, class Key>
__attribute__((target("avx512f"))) int find_avx512(const Key *data, int n, Key value) {
	unsigned less = 0;
	constexpr int lanes = 64 / sizeof(Key);
	for (int i = 0; i < n; i += lanes) {
		auto keys = _mm512_loadu_si512(data + i);
		unsigned match;
		if constexpr (sizeof(Key) == 4) {
			auto limit = _mm512_set1_epi32(static_cast<int>(value));
			if constexpr (std::is_unsigned_v<Key>) match = _mm512_cmp_epu32_mask(keys, limit, Strict ? _MM_CMPINT_LE : _MM_CMPINT_LT);
			else match = _mm512_cmp_epi32_mask(keys, limit, Strict ? _MM_CMPINT_LE : _MM_CMPINT_LT);
		} else {
			auto limit = _mm512_set1_epi64(static_cast<long long>(value));
			if constexpr (std::is_unsigned_v<Key>) match = _mm512_cmp_epu64_mask(keys, limit, Strict ? _MM_CMPINT_LE : _MM_CMPINT_LT);
			else match = _mm512_cmp_epi64_mask(keys, limit, Strict ? _MM_CMPINT_LE : _MM_CMPINT_LT);
		}
		less |= match << i;
	}
	if (n < 32) less &= (1U << n) - 1;
	return std::countr_one(less);
}
#endif
template<bool Strict, class Key>
int find(const Key *data, int n, Key value) {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	if constexpr (sizeof(Key) == 4 || sizeof(Key) == 8) {
		static const int kernel = __builtin_cpu_supports("avx512f") ? 2 : __builtin_cpu_supports("avx2") ? 1 : 0;
		if (n >= 8) {
			if (kernel == 2) return find_avx512<Strict>(data, n, value);
			if (kernel == 1) return find_avx2<Strict>(data, n, value);
		}
	}
#endif
	int l = 0, r = n;
	while (l < r) {
		int m = (l + r) / 2;
		if (Strict ? data[m] <= value : data[m] < value) l = m + 1;
		else r = m;
	}
	return l;
}

template<class T, class Sum, class Aggregate>
class BTree {
	static constexpr int branch = 16, capacity = 32;
	using Count = std::int64_t;
	struct Leaf {
		std::array<T, capacity + 1> keys{};
		std::array<Count, capacity + 1> copies{};
		Aggregate total = 0;
		Count size = 0;
		int used = 0;
	};
	struct Node {
		std::array<T, branch + 1> high{};
		std::array<int, branch + 1> child{};
		std::array<Count, branch + 1> sizes{};
		std::array<Aggregate, branch + 1> sums{};
		int used = 0;
	};
	std::vector<Leaf> leaves;
	std::vector<Node> nodes;
	int root = 0, first = -1, last = -1, free_leaf = -1, free_node = 0;
	static Aggregate weight(T value, Count count) { return static_cast<Aggregate>(static_cast<Sum>(value)) * count; }
	Count length(int p) const { return p < 0 ? leaves[-p - 1].size : nodes[p - 1].sizes[nodes[p - 1].used - 1]; }
	Aggregate total(int p) const { return p < 0 ? leaves[-p - 1].total : nodes[p - 1].sums[nodes[p - 1].used - 1]; }
	T maximum(int p) const { return p < 0 ? leaves[-p - 1].keys[leaves[-p - 1].used - 1] : nodes[p - 1].high[nodes[p - 1].used - 1]; }
	int used(int p) const { return p < 0 ? leaves[-p - 1].used : nodes[p - 1].used; }
	int make_leaf() {
		if (free_leaf >= 0) {
			int p = free_leaf;
			free_leaf = leaves[p].used;
			leaves[p].used = 0;
			leaves[p].size = 0;
			leaves[p].total = 0;
			return -p - 1;
		}
		assert(leaves.size() < std::size_t((std::numeric_limits<int>::max)()));
		leaves.emplace_back();
		return -static_cast<int>(leaves.size());
	}
	int make_node() {
		if (free_node) {
			int p = free_node;
			free_node = nodes[p - 1].used;
			nodes[p - 1].used = 0;
			return p;
		}
		assert(nodes.size() < std::size_t((std::numeric_limits<int>::max)()));
		nodes.emplace_back();
		return static_cast<int>(nodes.size());
	}
	void release(int p) {
		if (p < 0) {
			int i = -p - 1;
			leaves[i].used = free_leaf;
			free_leaf = i;
		} else {
			nodes[p - 1].used = free_node;
			free_node = p;
		}
	}
	void pull(int p) {
		auto &node = nodes[p - 1];
		Count size = 0;
		Aggregate sum = 0;
		for (int i = 0; i < node.used; ++i) {
			node.high[i] = maximum(node.child[i]);
			node.sizes[i] = size += length(node.child[i]);
			node.sums[i] = sum += total(node.child[i]);
		}
	}
	void pull_leaf(int p) {
		auto &leaf = leaves[-p - 1];
		leaf.size = 0; leaf.total = 0;
		for (int i = 0; i < leaf.used; ++i) {
			leaf.size += leaf.copies[i];
			leaf.total += weight(leaf.keys[i], leaf.copies[i]);
		}
	}
	int child(int p, T value) const {
		const auto &node = nodes[p - 1];
		if (value <= node.high[0]) return 0;
		if (node.high[node.used - 2] < value) return node.used - 1;
		return find<false>(node.high.data(), node.used - 1, value);
	}
	static int position(const Leaf &leaf, T value) {
		if (!leaf.used || value <= leaf.keys[0]) return 0;
		if (leaf.keys[leaf.used - 1] <= value) return leaf.used - 1 + (leaf.keys[leaf.used - 1] < value);
		return find<false>(leaf.keys.data(), leaf.used, value);
	}
	void add(int p, int j, Count delta, Aggregate difference) {
		auto &node = nodes[p - 1];
		node.high[j] = maximum(node.child[j]);
		for (int i = j; i < node.used; ++i) {
			node.sizes[i] += delta;
			node.sums[i] += difference;
		}
	}
	void prepare_split(const std::array<int, 32> &path, int depth) {
		constexpr std::size_t limit = (std::numeric_limits<int>::max)();
		if (free_leaf < 0 && leaves.size() == leaves.capacity()) {
			assert(leaves.size() < limit);
			leaves.reserve((std::min)(limit, leaves.capacity() + leaves.capacity() / 2 + 1));
		}
		std::size_t needed = 0;
		while (depth && nodes[path[depth - 1] - 1].used == branch) { ++needed; --depth; }
		if (!depth) ++needed;
		for (int p = free_node; p && needed; p = nodes[p - 1].used) --needed;
		if (needed > nodes.capacity() - nodes.size()) {
			assert(needed <= limit - nodes.size());
			nodes.reserve((std::max)(nodes.size() + needed, (std::min)(limit, nodes.capacity() + nodes.capacity() / 2 + 1)));
		}
	}
	bool rebalance(int p, int slot) {
		auto &parent = nodes[p - 1];
		int j = slot ? slot - 1 : 0;
		int a = parent.child[j], b = parent.child[j + 1];
		int n = used(a) + used(b);
		bool merge;
		if (a < 0) {
			std::array<T, capacity * 2> keys;
			std::array<Count, capacity * 2> copies;
			auto &left = leaves[-a - 1], &right = leaves[-b - 1];
			std::copy_n(left.keys.begin(), left.used, keys.begin());
			std::copy_n(right.keys.begin(), right.used, keys.begin() + left.used);
			std::copy_n(left.copies.begin(), left.used, copies.begin());
			std::copy_n(right.copies.begin(), right.used, copies.begin() + left.used);
			merge = n <= capacity;
			left.used = merge ? n : n / 2;
			std::copy_n(keys.begin(), left.used, left.keys.begin());
			std::copy_n(copies.begin(), left.used, left.copies.begin());
			pull_leaf(a);
			if (!merge) {
				right.used = n - left.used;
				std::copy_n(keys.begin() + left.used, right.used, right.keys.begin());
				std::copy_n(copies.begin() + left.used, right.used, right.copies.begin());
				pull_leaf(b);
			} else if (last == -b - 1) last = -a - 1;
		} else {
			std::array<int, branch * 2> children;
			auto &left = nodes[a - 1], &right = nodes[b - 1];
			std::copy_n(left.child.begin(), left.used, children.begin());
			std::copy_n(right.child.begin(), right.used, children.begin() + left.used);
			merge = n <= branch;
			left.used = merge ? n : n / 2;
			std::copy_n(children.begin(), left.used, left.child.begin());
			pull(a);
			if (!merge) {
				right.used = n - left.used;
				std::copy_n(children.begin() + left.used, right.used, right.child.begin());
				pull(b);
			}
		}
		if (merge) {
			release(b);
			std::move(parent.child.begin() + j + 2, parent.child.begin() + parent.used, parent.child.begin() + j + 1);
			--parent.used;
		}
		pull(p);
		return merge;
	}
public:
	BTree() = default;
	BTree(const BTree &) = default;
	BTree(BTree &&other) noexcept { swap(other); }
	BTree &operator=(BTree other) noexcept { swap(other); return *this; }
	Count size() const { return root ? length(root) : 0; }
	void clear() { leaves.clear(); nodes.clear(); root = free_node = 0; first = last = free_leaf = -1; }
	void reserve(std::size_t distinct) {
		std::size_t count = distinct / (capacity / 2) + 2;
		leaves.reserve(count); nodes.reserve(count / (branch / 2 - 1) + 2);
	}
	void assign(std::vector<T> values) {
		clear();
		if (values.empty()) return;
		stable_radix_sort(values, [](T value) { using U = std::make_unsigned_t<T>; constexpr U sign = std::is_signed_v<T> ? U(1) << (sizeof(T) * 8 - 1) : 0; return U(U(value) ^ sign); });
		std::size_t distinct = 0;
		std::vector<Count> counts;
		for (T value : values) {
			if (!distinct || values[distinct - 1] != value) { values[distinct++] = value; counts.push_back(1); }
			else ++counts.back();
		}
		assert(distinct <= std::size_t((std::numeric_limits<int>::max)()));
		int count = static_cast<int>((distinct + capacity * 3 / 4 - 1) / (capacity * 3 / 4));
		leaves.reserve(count);
		std::vector<int> layer;
		for (int i = 0, offset = 0; i < count; ++i) {
			int id = make_leaf();
			auto &leaf = leaves[-id - 1];
			leaf.used = static_cast<int>(distinct / count + (std::size_t(i) < distinct % count));
			std::copy_n(values.begin() + offset, leaf.used, leaf.keys.begin());
			std::copy_n(counts.begin() + offset, leaf.used, leaf.copies.begin());
			offset += leaf.used;
			pull_leaf(id); layer.push_back(id);
		}
		first = 0; last = count - 1;
		while (layer.size() > 1) {
			int count = static_cast<int>((layer.size() + branch - 1) / branch);
			std::vector<int> next;
			for (int i = 0, offset = 0; i < count; ++i) {
				int id = make_node();
				auto &node = nodes[id - 1];
				node.used = static_cast<int>(layer.size() / count + (std::size_t(i) < layer.size() % count));
				std::copy_n(layer.begin() + offset, node.used, node.child.begin());
				offset += node.used; pull(id); next.push_back(id);
			}
			layer.swap(next);
		}
		root = layer[0];
	}
	void insert(T value, Count count) {
		std::array<int, 32> path, slots;
		int p = root, depth = 0;
		if (!p) { root = p = make_leaf(); first = last = -p - 1; }
		while (p > 0) {
			int j = child(p, value);
			path[depth] = p; slots[depth++] = j; p = nodes[p - 1].child[j];
		}
		int index = -p - 1;
		int j = position(leaves[index], value), split = 0;
		bool present = j < leaves[index].used && leaves[index].keys[j] == value;
		if (!present && leaves[index].used == capacity) prepare_split(path, depth);
		auto &leaf = leaves[index];
		Aggregate difference = weight(value, count);
		leaf.size += count; leaf.total += difference;
		if (present) leaf.copies[j] += count;
		else {
			std::move_backward(leaf.keys.begin() + j, leaf.keys.begin() + leaf.used, leaf.keys.begin() + leaf.used + 1);
			std::move_backward(leaf.copies.begin() + j, leaf.copies.begin() + leaf.used, leaf.copies.begin() + leaf.used + 1);
			leaf.keys[j] = value; leaf.copies[j] = count; ++leaf.used;
			if (leaf.used > capacity) {
				split = make_leaf();
				auto &left = leaves[-p - 1], &right = leaves[-split - 1];
				left.used = capacity / 2; right.used = capacity / 2 + 1;
				std::copy_n(left.keys.begin() + left.used, right.used, right.keys.begin());
				std::copy_n(left.copies.begin() + left.used, right.used, right.copies.begin());
				pull_leaf(p); pull_leaf(split);
				if (last == -p - 1) last = -split - 1;
			}
		}
		while (depth) {
			p = path[--depth]; j = slots[depth];
			if (!split) { add(p, j, count, difference); continue; }
			auto &node = nodes[p - 1];
			std::move_backward(node.child.begin() + j + 1, node.child.begin() + node.used, node.child.begin() + node.used + 1);
			node.child[j + 1] = split; ++node.used;
			split = 0;
			if (node.used > branch) {
				split = make_node();
				auto &left = nodes[p - 1], &right = nodes[split - 1];
				left.used = branch / 2; right.used = branch / 2 + 1;
				std::copy_n(left.child.begin() + left.used, right.used, right.child.begin());
				pull(split);
			}
			pull(p);
		}
		if (split) {
			int next = make_node();
			nodes[next - 1].used = 2; nodes[next - 1].child[0] = root; nodes[next - 1].child[1] = split;
			pull(next); root = next;
		}
	}
	Count erase(T value, Count count) {
		if (!root) return 0;
		std::array<int, 32> path, slots;
		int p = root, depth = 0;
		while (p > 0) {
			int j = child(p, value);
			path[depth] = p; slots[depth++] = j; p = nodes[p - 1].child[j];
		}
		auto &leaf = leaves[-p - 1];
		int j = position(leaf, value);
		if (j == leaf.used || leaf.keys[j] != value) return 0;
		count = (std::min)(count, leaf.copies[j]);
		Aggregate difference = -weight(value, count);
		leaf.size -= count; leaf.total += difference; leaf.copies[j] -= count;
		if (!leaf.copies[j]) {
			std::move(leaf.keys.begin() + j + 1, leaf.keys.begin() + leaf.used, leaf.keys.begin() + j);
			std::move(leaf.copies.begin() + j + 1, leaf.copies.begin() + leaf.used, leaf.copies.begin() + j);
			--leaf.used;
		}
		if (!depth) {
			if (!leaf.used) { release(root); root = 0; first = last = -1; }
			return count;
		}
		bool small = leaf.used < capacity / 2;
		while (depth) {
			p = path[--depth]; j = slots[depth];
			if (small) { rebalance(p, j); small = nodes[p - 1].used < branch / 2; }
			else add(p, j, -count, difference);
		}
		if (root > 0 && nodes[root - 1].used == 1) { int old = root; root = nodes[old - 1].child[0]; release(old); }
		return count;
	}
	Count count(T value) const {
		int p = root;
		if (!p) return 0;
		while (p > 0) p = nodes[p - 1].child[child(p, value)];
		const auto &leaf = leaves[-p - 1];
		int j = position(leaf, value);
		return j < leaf.used && leaf.keys[j] == value ? leaf.copies[j] : 0;
	}
	template<bool Strict = false> Count order_of_key(T value) const {
		Count result = 0;
		int p = root;
		if (!p) return 0;
		while (p > 0) {
			const auto &node = nodes[p - 1]; int j = child(p, value);
			if (j) result += node.sizes[j - 1];
			p = node.child[j];
		}
		const auto &leaf = leaves[-p - 1];
		int j = find<Strict>(leaf.keys.data(), leaf.used, value);
		for (int i = 0; i < j; ++i) result += leaf.copies[i];
		return result;
	}
	T kth(Count k) const {
		if (!k) return leaves[first].keys[0];
		if (k == size() - 1) return leaves[last].keys[leaves[last].used - 1];
		int p = root;
		while (p > 0) {
			const auto &node = nodes[p - 1];
			int j = find<true>(node.sizes.data(), node.used, k);
			if (j) k -= node.sizes[j - 1];
			p = node.child[j];
		}
		const auto &leaf = leaves[-p - 1];
		int j = 0;
		while (k >= leaf.copies[j]) k -= leaf.copies[j++];
		return leaf.keys[j];
	}
	Aggregate sum(Count k) const {
		if (!root || !k) return 0;
		if (k >= size()) return total(root);
		Aggregate answer = 0;
		int p = root;
		while (p > 0) {
			const auto &node = nodes[p - 1];
			int j = find<false>(node.sizes.data(), node.used, k);
			if (j) { k -= node.sizes[j - 1]; answer += node.sums[j - 1]; } p = node.child[j];
		}
		const auto &leaf = leaves[-p - 1];
		for (int i = 0; k; ++i) { Count take = (std::min)(k, leaf.copies[i]); answer += weight(leaf.keys[i], take); k -= take; }
		return answer;
	}
	Aggregate sum_all() const { return root ? total(root) : Aggregate(0); }
	template<bool Strict> std::optional<T> bound(T value) const {
		int p = root;
		if (!p) return std::nullopt;
		while (p > 0) {
			const auto &node = nodes[p - 1];
			int j = find<Strict>(node.high.data(), node.used, value);
			if (j == node.used) return std::nullopt;
			p = node.child[j];
		}
		const auto &leaf = leaves[-p - 1];
		int j = find<Strict>(leaf.keys.data(), leaf.used, value);
		return j == leaf.used ? std::nullopt : std::optional<T>(leaf.keys[j]);
	}
	void swap(BTree &other) noexcept {
		leaves.swap(other.leaves); nodes.swap(other.nodes);
		std::swap(root, other.root); std::swap(first, other.first); std::swap(last, other.last);
		std::swap(free_leaf, other.free_leaf); std::swap(free_node, other.free_node);
	}
};
}

#endif
