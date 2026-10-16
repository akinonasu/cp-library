#ifndef DYNAMIC_SEGTREE_BLOCK_HPP
#define DYNAMIC_SEGTREE_BLOCK_HPP
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, class Coord = std::int64_t, int L = 16, int F = 4, bool Balanced = false>
class DynamicSegtreeBlock {
	static_assert(L >= 4 && F >= 4);
	using U = std::make_unsigned_t<Coord>;
	static constexpr unsigned tag = 1u << 31;
	static constexpr U sign = std::is_signed_v<Coord> ? U(1) << (std::numeric_limits<U>::digits - 1) : U(0);
	template <std::size_t... I>
	static auto units(std::index_sequence<I...>) { return std::array<S, sizeof...(I)>{(void(I), e())...}; }
	struct Leaf {
		std::array<U, L + 1> keys{};
		std::array<S, L + 1> values = units(std::make_index_sequence<L + 1>{});
		S product = e();
		unsigned count = 0;
	};
	struct Branch {
		std::array<U, F + 1> keys{};
		std::array<unsigned, F + 1> children{};
		std::array<S, F + 1> products = units(std::make_index_sequence<F + 1>{});
		S product = e();
		unsigned count = 0;
	};
	std::vector<Leaf> leaves;
	std::vector<Branch> branches;
	unsigned root = 0, points = 0;
	Coord first, last;
	static U encode(Coord x) { return U(x) ^ sign; }
	static Coord decode(U x) { return Coord(x ^ sign); }
	Leaf &leaf(unsigned id) { return leaves[id & ~tag]; }
	const Leaf &leaf(unsigned id) const { return leaves[id & ~tag]; }
	Branch &branch(unsigned id) { return branches[id - 1]; }
	const Branch &branch(unsigned id) const { return branches[id - 1]; }
	const S &product(unsigned id) const { return id & tag ? leaf(id).product : branch(id).product; }
	U maximum(unsigned id) const {
		if (id & tag) return leaf(id).keys[leaf(id).count - 1];
		return branch(id).keys[branch(id).count - 1];
	}
	unsigned make_leaf() {
		leaves.emplace_back();
		return unsigned(leaves.size() - 1) | tag;
	}
	unsigned make_branch() {
		branches.emplace_back();
		return unsigned(branches.size());
	}
	template <unsigned N>
	[[gnu::always_inline]] static inline S fold(const S *values, unsigned count) {
		if constexpr (N == 1) return values[0];
		else {
			if (count <= N / 2) return fold<N / 2>(values, count);
			return op(fold<N / 2>(values, N / 2), fold<N / 2>(values + N / 2, count - N / 2));
		}
	}
	template <class Node>
	static void pull(Node &node) {
		if constexpr (Balanced) node.product = fold<F>(node.products.data(), node.count);
		else {
			S value = node.products[0];
			for (unsigned i = 1; i < node.count; ++i) value = op(value, node.products[i]);
			node.product = std::move(value);
		}
	}
	static void pull(Leaf &node) {
		if constexpr (Balanced) node.product = fold<L>(node.values.data(), node.count);
		else {
			S value = node.values[0];
			for (unsigned i = 1; i < node.count; ++i) value = op(value, node.values[i]);
			node.product = std::move(value);
		}
	}
	void refresh(unsigned id, unsigned position, unsigned child) {
		auto &node = branch(id);
		node.keys[position] = maximum(child);
		node.children[position] = child;
		node.products[position] = product(child);
	}
	template <bool Append>
	void update(U key, S value) {
		std::array<unsigned, 64> path, positions;
		unsigned depth = 0, id = root;
		if (!id) id = root = make_leaf();
		while (!(id & tag)) {
			const auto &node = branch(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count - 1, key) - node.keys.begin();
			path[depth] = id;
			positions[depth++] = p;
			id = node.children[p];
		}
		auto &node = leaf(id);
		unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, key) - node.keys.begin();
		if (p < node.count && node.keys[p] == key) {
			if constexpr (Append) node.values[p] = op(node.values[p], value);
			else node.values[p] = std::move(value);
		} else {
			std::move_backward(node.keys.begin() + p, node.keys.begin() + node.count, node.keys.begin() + node.count + 1);
			std::move_backward(node.values.begin() + p, node.values.begin() + node.count, node.values.begin() + node.count + 1);
			node.keys[p] = key;
			node.values[p] = std::move(value);
			++node.count;
			++points;
		}
		unsigned split = 0;
		if (node.count > L) {
			split = make_leaf();
			auto &a = leaf(id), &b = leaf(split);
			unsigned middle = a.count / 2;
			b.count = a.count - middle;
			for (unsigned i = 0; i < b.count; ++i) b.keys[i] = a.keys[middle + i], b.values[i] = std::move(a.values[middle + i]);
			a.count = middle;
			pull(a);
			pull(b);
		} else pull(node);
		while (depth) {
			unsigned parent = path[--depth], index = positions[depth];
			refresh(parent, index, id);
			auto &current = branch(parent);
			if (split) {
				for (unsigned i = current.count; i > index + 1; --i) {
					current.keys[i] = current.keys[i - 1];
					current.children[i] = current.children[i - 1];
					current.products[i] = std::move(current.products[i - 1]);
				}
				++current.count;
				refresh(parent, index + 1, split);
			}
			id = parent;
			split = 0;
			if (current.count > F) {
				split = make_branch();
				auto &a = branch(id), &b = branch(split);
				unsigned middle = a.count / 2;
				b.count = a.count - middle;
				for (unsigned i = 0; i < b.count; ++i) {
					b.keys[i] = a.keys[middle + i];
					b.children[i] = a.children[middle + i];
					b.products[i] = std::move(a.products[middle + i]);
				}
				a.count = middle;
				pull(a);
				pull(b);
			} else pull(current);
		}
		if (split) {
			root = make_branch();
			branch(root).count = 2;
			refresh(root, 0, id);
			refresh(root, 1, split);
			pull(branch(root));
		}
	}
	S range(unsigned id, U l, U r, U lower) const {
		if (l <= lower && maximum(id) < r) return product(id);
		S result = e();
		if (id & tag) {
			const auto &node = leaf(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, l) - node.keys.begin();
			for (; p < node.count && node.keys[p] < r; ++p) result = op(result, node.values[p]);
		} else {
			const auto &node = branch(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, l) - node.keys.begin();
			for (; p < node.count; ++p) {
				U start = p ? U(node.keys[p - 1] + 1) : lower;
				if (r <= start) break;
				if (l <= start && node.keys[p] < r) result = op(result, node.products[p]);
				else result = op(result, range(node.children[p], l, r, start));
			}
		}
		return result;
	}
	template <class Pred>
	bool search_right(unsigned id, U lower, U l, S &value, Pred &f, Coord &answer) const {
		if (maximum(id) < l) return false;
		if (l <= lower) {
			S next = op(value, product(id));
			if (f(next)) { value = std::move(next); return false; }
		}
		if (id & tag) {
			const auto &node = leaf(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, l) - node.keys.begin();
			for (; p < node.count; ++p) {
				S next = op(value, node.values[p]);
				if (!f(next)) { answer = decode(node.keys[p]); return true; }
				value = std::move(next);
			}
		} else {
			const auto &node = branch(id);
			for (unsigned p = 0; p < node.count; ++p)
				if (search_right(node.children[p], p ? U(node.keys[p - 1] + 1) : lower, l, value, f, answer)) return true;
		}
		return false;
	}
	template <class Pred>
	bool search_left(unsigned id, U lower, U r, S &value, Pred &f, Coord &answer) const {
		if (r <= lower) return false;
		if (maximum(id) < r) {
			S next = op(product(id), value);
			if (f(next)) { value = std::move(next); return false; }
		}
		if (id & tag) {
			const auto &node = leaf(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, r) - node.keys.begin();
			while (p) {
				--p;
				S next = op(node.values[p], value);
				if (!f(next)) { answer = decode(node.keys[p]) + 1; return true; }
				value = std::move(next);
			}
		} else {
			const auto &node = branch(id);
			for (unsigned p = node.count; p-- > 0;)
				if (search_left(node.children[p], p ? U(node.keys[p - 1] + 1) : lower, r, value, f, answer)) return true;
		}
		return false;
	}

public:
	using value_type = S;
	using coordinate_type = Coord;
	explicit DynamicSegtreeBlock(Coord n = 0) : DynamicSegtreeBlock(0, n) {}
	DynamicSegtreeBlock(Coord l, Coord r) : first(l), last(r) { assert(l <= r); }
	DynamicSegtreeBlock(const DynamicSegtreeBlock &) = default;
	DynamicSegtreeBlock &operator=(const DynamicSegtreeBlock &) = default;
	DynamicSegtreeBlock(DynamicSegtreeBlock &&other) : DynamicSegtreeBlock(other.first, other.last) { swap(other); }
	DynamicSegtreeBlock &operator=(DynamicSegtreeBlock &&other) {
		if (this != &other) { DynamicSegtreeBlock moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(DynamicSegtreeBlock &other) noexcept {
		leaves.swap(other.leaves);
		branches.swap(other.branches);
		std::swap(root, other.root);
		std::swap(points, other.points);
		std::swap(first, other.first);
		std::swap(last, other.last);
	}
	void reserve(std::size_t n) {
		leaves.reserve(n / (L / 2) + 1);
		branches.reserve(n / (L / 2) / (F / 2 - 1) + 1);
	}
	std::size_t node_count() const { return points; }
	std::size_t memory_usage() const { return leaves.capacity() * sizeof(Leaf) + branches.capacity() * sizeof(Branch); }
	U size() const { return U(last) - U(first); }
	Coord left_bound() const { return first; }
	Coord right_bound() const { return last; }
	void clear() { leaves.clear(); branches.clear(); root = points = 0; }
	void set(Coord p, S value) { assert(first <= p && p < last); update<false>(encode(p), std::move(value)); }
	void multiply(Coord p, S value) { assert(first <= p && p < last); update<true>(encode(p), std::move(value)); }
	S get(Coord p) const {
		assert(first <= p && p < last);
		if (!root) return e();
		U key = encode(p);
		unsigned id = root;
		while (!(id & tag)) {
			const auto &node = branch(id);
			unsigned i = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count - 1, key) - node.keys.begin();
			id = node.children[i];
		}
		const auto &node = leaf(id);
		unsigned i = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, key) - node.keys.begin();
		return i < node.count && node.keys[i] == key ? node.values[i] : e();
	}
	S prod(Coord l, Coord r) const {
		assert(first <= l && l <= r && r <= last);
		if (!root || l == r) return e();
		return range(root, encode(l), encode(r), encode(first));
	}
	S all_prod() const { return root ? product(root) : e(); }
	template <class Pred>
	Coord max_right(Coord l, Pred f) const {
		assert(first <= l && l <= last && f(e()));
		Coord answer = last;
		if (root && l != last) { S value = e(); search_right(root, encode(first), encode(l), value, f, answer); }
		return answer;
	}
	template <class Pred>
	Coord min_left(Coord r, Pred f) const {
		assert(first <= r && r <= last && f(e()));
		Coord answer = first;
		if (root && r != first) { S value = e(); search_left(root, encode(first), encode(r), value, f, answer); }
		return answer;
	}
};
#endif
