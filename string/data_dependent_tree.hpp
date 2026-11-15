#ifndef DATA_DEPENDENT_TREE_HPP
#define DATA_DEPENDENT_TREE_HPP

#include "../components/splitmix64.hpp"
#include "../components/sequence_view.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

template <class Symbol>
class BasicDataDependentTree {
	static_assert(std::is_integral_v<Symbol> && sizeof(Symbol) <= 8 && !std::is_same_v<Symbol, bool>);
	static constexpr bool byte = std::is_same_v<Symbol, char>;
	using View = std::conditional_t<byte, std::string_view, std::span<const Symbol>>;
	using Sequence = std::conditional_t<byte, std::string, std::vector<Symbol>>;
	struct Empty {
		explicit Empty(std::uint64_t = 0) {}
		void swap(Empty &) noexcept {}
	};
	struct SymbolHash {
		std::uint64_t seed;
		std::size_t operator()(Symbol value) const noexcept { return splitmix64(std::uint64_t(value) + seed); }
	};
	struct Leaves {
		std::unordered_map<Symbol, int, SymbolHash> index;
		explicit Leaves(std::uint64_t seed) : index(0, SymbolHash{seed}) {}
		void swap(Leaves &other) noexcept { index.swap(other.index); }
	};
	using Size = std::uint64_t;
	struct Node {
		Size length = 0, count = 0, priority = 0;
		int level = -1, child = 0;
		std::vector<int> children;
		std::vector<Size> prefix;
		[[no_unique_address]] std::conditional_t<byte, Empty, Symbol> symbol{};
	};
	struct Part { int node; Size count; };
	using Forest = std::vector<Part>;
	std::vector<Node> nodes_;
	std::unordered_multimap<Size, int> signatures_;
	Size seed_;
	[[no_unique_address]] std::conditional_t<byte, Empty, Leaves> leaves_;
	static Size random_seed() {
		auto now = std::chrono::steady_clock::now().time_since_epoch().count();
		return splitmix64(Size(now) ^ Size(reinterpret_cast<std::uintptr_t>(&now)));
	}
	void check([[maybe_unused]] int v) const { assert(0 <= v && std::size_t(v) < nodes_.size()); }
	static void push(Forest &out, int v, Size count = 1) {
		if (!count) return;
		if (!out.empty() && out.back().node == v) out.back().count += count;
		else out.push_back({v, count});
	}
	static int pop(Forest &out) {
		int v = out.back().node;
		if (!--out.back().count) out.pop_back();
		return v;
	}
	int intern(int level, int child, Size count, std::vector<int> children = {}) {
		Size hash = splitmix64(seed_ + Size(level));
		hash = splitmix64(hash ^ count);
		hash = splitmix64(hash ^ unsigned(child));
		for (int v : children) hash = splitmix64(hash ^ unsigned(v));
		auto [first, last] = signatures_.equal_range(hash);
		for (; first != last; ++first) {
			const auto &x = nodes_[first->second];
			if (x.level == level && x.child == child && x.count == count && x.children == children) return first->second;
		}
		if (nodes_.size() >= std::size_t(std::numeric_limits<int>::max())) throw std::length_error("DataDependentTree nodes");
		int id = int(nodes_.size());
		Node x;
		x.level = level, x.child = child, x.count = count, x.priority = splitmix64(seed_ + unsigned(id));
		if (child) {
			assert(count && nodes_[child].length <= std::numeric_limits<Size>::max() / count);
			x.length = nodes_[child].length * count;
		} else {
			x.children = std::move(children);
			x.prefix.push_back(0);
			for (int v : x.children) {
				assert(nodes_[v].length <= std::numeric_limits<Size>::max() - x.length);
				x.length += nodes_[v].length;
				x.prefix.push_back(x.length);
			}
		}
		nodes_.push_back(std::move(x));
		try { signatures_.emplace(hash, id); }
		catch (...) { nodes_.pop_back(); throw; }
		return id;
	}
	int character(Symbol value) {
		if constexpr (byte) return int(static_cast<unsigned char>(value)) + 1;
		else {
			auto it = leaves_.index.find(value);
			if (it != leaves_.index.end()) return it->second;
			if (nodes_.size() >= std::size_t(std::numeric_limits<int>::max())) throw std::length_error("DataDependentTree nodes");
			int id = int(nodes_.size());
			Node x;
			x.length = x.count = 1;
			x.level = 0;
			x.priority = splitmix64(seed_ + unsigned(id));
			x.symbol = value;
			nodes_.push_back(std::move(x));
			try { leaves_.index.emplace(value, id); }
			catch (...) { nodes_.pop_back(); throw; }
			return id;
		}
	}
	Symbol symbol(int v) const {
		if constexpr (byte) return static_cast<char>(v - 1);
		else return nodes_[v].symbol;
	}
	int run(int v, Size count, int level) { return intern(level, v, count); }
	int sequence(std::vector<int> children, int level) {
		Size count = children.size();
		return intern(level, 0, count, std::move(children));
	}
	int trim(int v) const {
		while (nodes_[v].level > 0 && nodes_[v].count == 1)
			v = nodes_[v].child ? nodes_[v].child : nodes_[v].children[0];
		return v;
	}
	void children(int v, Forest &out, bool reverse) const {
		const auto &x = nodes_[v];
		assert(x.level > 0);
		if (x.child) push(out, x.child, x.count);
		else if (reverse) for (auto it = x.children.rbegin(); it != x.children.rend(); ++it) push(out, *it);
		else for (int c : x.children) push(out, c);
	}
	void expand(Forest &out, bool reverse) const {
		int v = pop(out);
		children(v, out, reverse);
	}
	Forest boundary(Forest &side, int level, bool left) const {
		Forest result;
		while (!side.empty() && nodes_[side.back().node].level == level) {
			push(result, side.back().node, side.back().count);
			side.pop_back();
		}
		if (!side.empty()) {
			while (nodes_[side.back().node].level > level + 1) expand(side, !left);
			int v = pop(side);
			assert(nodes_[v].level == level + 1);
			children(v, result, left);
		}
		if (left) std::reverse(result.begin(), result.end());
		return result;
	}
	Forest group(const Forest &input, int level) {
		Forest result;
		if (level & 1) {
			for (auto part : input) push(result, run(part.node, part.count, level));
		} else {
			std::vector<int> current;
			auto flush = [&] {
				if (!current.empty()) {
					push(result, sequence(std::move(current), level));
					current.clear();
				}
			};
			for (auto [v, count] : input) {
				if (!current.empty() && nodes_[current.back()].priority >= nodes_[v].priority) flush();
				current.push_back(v);
				if (count > 1) {
					flush();
					if (count > 2) push(result, sequence(std::vector<int>{v}, level), count - 2);
					current.push_back(v);
				}
			}
			flush();
		}
		return result;
	}
	int repair(Forest left, Forest right = {}) {
		Forest middle;
		for (int level = 0;; ++level) {
			if (middle.empty() && left.empty() && right.empty()) return 0;
			if (middle.empty() && left.empty() && right.size() == 1 && right[0].count == 1) return trim(right[0].node);
			if (middle.empty() && right.empty() && left.size() == 1 && left[0].count == 1) return trim(left[0].node);
			auto before = boundary(left, level, true), after = boundary(right, level, false);
			for (auto part : middle) push(before, part.node, part.count);
			for (auto part : after) push(before, part.node, part.count);
			if (left.empty() && right.empty() && before.size() == 1 && before[0].count == 1) return trim(before[0].node);
			middle = group(before, level + 1);
		}
	}
	void check_sum(int a, int b) const {
		if (nodes_[b].length > std::numeric_limits<Size>::max() - nodes_[a].length) throw std::length_error("DataDependentTree length");
	}
	Forest cursor(int root, Size p = 0) const {
		Forest out;
		if (p == nodes_[root].length) return out;
		out.reserve(std::size_t(nodes_[root].level) + 8);
		while (p) {
			const auto &x = nodes_[root];
			if (x.child) {
				Size width = nodes_[x.child].length, count = p / width;
				p %= width;
				push(out, x.child, x.count - count - bool(p));
				if (!p) return out;
				root = x.child;
			} else {
				std::size_t j = std::upper_bound(x.prefix.begin(), x.prefix.end(), p) - x.prefix.begin() - 1;
				for (std::size_t i = x.children.size(); i-- > j + 1;) push(out, x.children[i]);
				p -= x.prefix[j]; root = x.children[j];
			}
		}
		push(out, root);
		return out;
	}
	Size common_prefix(Forest left, Forest right, Size limit) const {
		Size matched = 0;
		while (matched < limit) {
			auto x = left.back(), y = right.back();
			if (x.node == y.node) {
				Size count = std::min(x.count, y.count), length = nodes_[x.node].length * count;
				if (length >= limit - matched) return limit;
				matched += length;
				if (!(left.back().count -= count)) left.pop_back();
				if (!(right.back().count -= count)) right.pop_back();
			} else {
				int hl = nodes_[x.node].level, hr = nodes_[y.node].level;
				if (!hl && !hr) return matched;
				if (hl >= hr) expand(left, true);
				if (hr >= hl) expand(right, true);
			}
		}
		return matched;
	}

  public:
	using size_type = Size;
	explicit BasicDataDependentTree(Size seed = random_seed()) : nodes_(byte ? 257 : 1), seed_(seed), leaves_(seed) {
		if constexpr (byte) for (int v = 1; v <= 256; ++v) {
			nodes_[v].length = nodes_[v].count = 1;
			nodes_[v].level = 0;
			nodes_[v].priority = splitmix64(seed_ + unsigned(v));
		}
	}
	BasicDataDependentTree(const BasicDataDependentTree &) = default;
	BasicDataDependentTree &operator=(const BasicDataDependentTree &other) {
		if (this != &other) { BasicDataDependentTree next(other); swap(next); }
		return *this;
	}
	BasicDataDependentTree(BasicDataDependentTree &&other) : BasicDataDependentTree(other.seed_) { swap(other); }
	BasicDataDependentTree &operator=(BasicDataDependentTree &&other) {
		if (this != &other) { BasicDataDependentTree next(std::move(other)); swap(next); }
		return *this;
	}
	void swap(BasicDataDependentTree &other) noexcept {
		nodes_.swap(other.nodes_); signatures_.swap(other.signatures_); std::swap(seed_, other.seed_); leaves_.swap(other.leaves_);
	}
	Size size(int root) const { check(root); return nodes_[root].length; }
	bool empty(int root) const { check(root); return !root; }
	int height(int root) const { check(root); return std::max(0, nodes_[root].level); }
	std::size_t storage_nodes() const { return nodes_.size() - 1; }
	void reserve(std::size_t count) {
		if (count >= std::size_t(std::numeric_limits<int>::max())) throw std::length_error("DataDependentTree nodes");
		nodes_.reserve(std::max(count + 1, std::size_t(byte ? 257 : 1)));
		signatures_.reserve(count);
		if constexpr (!byte) leaves_.index.reserve(count);
	}
	int build(View text) {
		Forest current;
		current.reserve(text.size());
		for (Symbol c : text) push(current, character(c));
		for (int level = 1; !current.empty(); ++level) {
			if (current.size() == 1 && current[0].count == 1) return trim(current[0].node);
			current = group(current, level);
		}
		return 0;
	}
	template <sequence_view_detail::CharSequence R> requires byte
	int build(const R &text) { return build(sequence_view_detail::char_view(text)); }
	int concat(int a, int b) {
		check(a); check(b); check_sum(a, b);
		if (!a || !b) return a ? a : b;
		return repair(Forest{{a, 1}}, Forest{{b, 1}});
	}
	std::pair<int, int> split(int root, Size p) {
		check(root); assert(p <= size(root));
		if (!p) return {0, root};
		if (p == size(root)) return {root, 0};
		Forest left, right;
		int v = root;
		while (nodes_[v].level > 0) {
			const auto &x = nodes_[v];
			if (x.child) {
				Size width = nodes_[x.child].length, count = p / width;
				push(left, x.child, count);
				p %= width;
				push(right, x.child, x.count - count - bool(p));
				if (!p) break;
				v = x.child;
			} else {
				std::size_t j = std::upper_bound(x.prefix.begin(), x.prefix.end(), p) - x.prefix.begin() - 1;
				for (std::size_t i = 0; i < j; ++i) push(left, x.children[i]);
				p -= x.prefix[j];
				for (std::size_t i = x.children.size(); i-- > j + 1;) push(right, x.children[i]);
				v = x.children[j];
				if (!p) { push(right, v); break; }
			}
		}
		int a = repair(std::move(left)), b = repair({}, std::move(right));
		return {a, b};
	}
	int substr(int root, Size l, Size r) {
		assert(l <= r && r <= size(root));
		if (l == r) return 0;
		int prefix = split(root, r).first;
		return split(prefix, l).second;
	}
	int insert(int root, Size p, int text) {
		check(root); check(text); assert(p <= size(root)); check_sum(root, text);
		if (!text) return root;
		auto [a, b] = split(root, p);
		return concat(concat(a, text), b);
	}
	int insert(int root, Size p, View text) { return insert(root, p, build(text)); }
	template <sequence_view_detail::CharSequence R> requires byte
	int insert(int root, Size p, const R &text) { return insert(root, p, sequence_view_detail::char_view(text)); }
	int erase(int root, Size l, Size r) {
		assert(l <= r && r <= size(root));
		if (l == r) return root;
		auto [a, rest] = split(root, l);
		int b = split(rest, r - l).second;
		return concat(a, b);
	}
	int set(int root, Size p, Symbol c) {
		assert(p < size(root));
		auto [a, rest] = split(root, p);
		int b = split(rest, 1).second;
		return concat(concat(a, character(c)), b);
	}
	int repeat(int root, Size count) {
		check(root);
		if (count && size(root) > std::numeric_limits<Size>::max() / count) throw std::length_error("DataDependentTree length");
		int result = 0;
		while (count) {
			if (count & 1) result = concat(result, root);
			count >>= 1;
			if (count) root = concat(root, root);
		}
		return result;
	}
	Symbol at(int root, Size p) const {
		assert(p < size(root));
		while (nodes_[root].level > 0) {
			const auto &x = nodes_[root];
			if (x.child) { root = x.child; p %= nodes_[root].length; }
			else {
				std::size_t j = std::upper_bound(x.prefix.begin(), x.prefix.end(), p) - x.prefix.begin() - 1;
				p -= x.prefix[j]; root = x.children[j];
			}
		}
		return symbol(root);
	}
	bool equal(int a, int b) const { check(a); check(b); return a == b; }
	Size lcp(int a, int b, Size limit = std::numeric_limits<Size>::max()) const {
		check(a); check(b);
		limit = std::min({limit, size(a), size(b)});
		if (!limit || a == b) return limit;
		return common_prefix(cursor(a), cursor(b), limit);
	}
	Size lcp(int a, Size p, int b, Size q, Size limit = std::numeric_limits<Size>::max()) const {
		check(a); check(b); assert(p <= size(a) && q <= size(b));
		limit = std::min({limit, size(a) - p, size(b) - q});
		if (!limit || (a == b && p == q)) return limit;
		return common_prefix(cursor(a, p), cursor(b, q), limit);
	}
	int compare(int a, int b) const {
		Size common = lcp(a, b);
		if (common == std::min(size(a), size(b))) return (size(a) > size(b)) - (size(a) < size(b));
		using Ordered = std::conditional_t<byte, unsigned char, Symbol>;
		Ordered x = at(a, common), y = at(b, common);
		return (x > y) - (x < y);
	}
	Sequence str(int root) const {
		check(root);
		Sequence result;
		if (size(root) > result.max_size()) throw std::length_error("DataDependentTree string");
		result.reserve(std::size_t(size(root)));
		auto pending = cursor(root);
		while (!pending.empty()) {
			int v = pending.back().node;
			if (!nodes_[v].level) {
				if constexpr (byte) result.append(std::size_t(pending.back().count), symbol(v));
				else result.insert(result.end(), std::size_t(pending.back().count), symbol(v));
				pending.pop_back();
			} else expand(pending, true);
		}
		return result;
	}
};

using DataDependentTree = BasicDataDependentTree<char>;

#endif
