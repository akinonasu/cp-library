#ifndef TRIE_HPP
#define TRIE_HPP

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace trie_detail {
struct Empty {};
}

template<int Alphabet = 26, int First = 'a', class Data = trie_detail::Empty>
class Trie {
	static_assert(0 < Alphabet && 0 <= First && First <= 255 && Alphabet <= 256 - First);
	struct Node {
		std::array<int, Alphabet> next;
		int parent, depth;
		char letter;
		std::int64_t count = 0, terminal = 0;
		[[no_unique_address]] Data data{};
		Node(int p = -1, int d = 0, char c = 0) : parent(p), depth(d), letter(c) { next.fill(-1); }
	};
	std::vector<Node> nodes_;
	int distinct_ = 0;
	static int symbol_(char c) { return int(static_cast<unsigned char>(c)) - First; }
	void check_([[maybe_unused]] int v) const { assert(0 <= v && v < node_count()); }
	void check_count_(std::int64_t copies) const {
		assert(copies > 0);
		if (copies > std::numeric_limits<std::int64_t>::max() - size()) throw std::overflow_error("Trie count");
	}
	int add_(int v, std::int64_t copies) {
		if (!nodes_[v].terminal) ++distinct_;
		nodes_[v].terminal += copies;
		for (int u = v; u != -1; u = nodes_[u].parent) nodes_[u].count += copies;
		return v;
	}

  public:
	static constexpr int root = 0;
	Trie() { nodes_.emplace_back(); }
	std::int64_t size() const { return nodes_[root].count; }
	bool empty() const { return size() == 0; }
	int distinct_size() const { return distinct_; }
	int node_count() const { return static_cast<int>(nodes_.size()); }
	void reserve(int total_length) {
		assert(0 <= total_length && total_length < std::numeric_limits<int>::max());
		nodes_.reserve(std::size_t(total_length) + 1);
	}
	void clear() { nodes_.clear(), nodes_.emplace_back(); distinct_ = 0; }
	int ensure(std::string_view s) {
		assert(s.size() < std::size_t(std::numeric_limits<int>::max()));
		int v = root;
		for (char c : s) {
			int x = symbol_(c);
			assert(0 <= x && x < Alphabet);
			int u = nodes_[v].next[x];
			if (u == -1) {
				assert(node_count() < std::numeric_limits<int>::max());
				u = node_count();
				nodes_.emplace_back(v, nodes_[v].depth + 1, c);
				nodes_[v].next[x] = u;
			}
			v = u;
		}
		return v;
	}
	int insert(std::string_view s, std::int64_t copies = 1) {
		check_count_(copies);
		return add_(ensure(s), copies);
	}
	int insert_node(int v, std::int64_t copies = 1) {
		check_(v), check_count_(copies);
		return add_(v, copies);
	}
	bool erase_node(int v, std::int64_t copies = 1) {
		check_(v), assert(copies > 0);
		if (nodes_[v].terminal < copies) return false;
		nodes_[v].terminal -= copies;
		if (!nodes_[v].terminal) --distinct_;
		for (int u = v; u != -1; u = nodes_[u].parent) nodes_[u].count -= copies;
		return true;
	}
	bool erase(std::string_view s, std::int64_t copies = 1) {
		assert(copies > 0);
		int v = find(s);
		return v != -1 && erase_node(v, copies);
	}
	int parent(int v) const { check_(v); return nodes_[v].parent; }
	int depth(int v) const { check_(v); return nodes_[v].depth; }
	char letter(int v) const { check_(v); assert(v != root); return nodes_[v].letter; }
	const std::array<int, Alphabet> &children(int v) const { check_(v); return nodes_[v].next; }
	int child(int v, char c) const {
		check_(v);
		int x = symbol_(c);
		return 0 <= x && x < Alphabet ? nodes_[v].next[x] : -1;
	}
	std::int64_t terminal_count(int v) const { check_(v); return nodes_[v].terminal; }
	std::int64_t prefix_count(int v) const { check_(v); return nodes_[v].count; }
	Data &data(int v) { check_(v); return nodes_[v].data; }
	const Data &data(int v) const { check_(v); return nodes_[v].data; }
	int walk(int v, std::string_view s) const {
		check_(v);
		for (char c : s) { v = child(v, c); if (v == -1) break; }
		return v;
	}
	int find(std::string_view s) const { return walk(root, s); }
	std::int64_t count(std::string_view s) const { int v = find(s); return v == -1 ? 0 : nodes_[v].terminal; }
	bool contains(std::string_view s) const { return count(s) != 0; }
	std::int64_t prefix_count(std::string_view s) const { int v = find(s); return v == -1 ? 0 : nodes_[v].count; }
	std::string restore(int v) const {
		check_(v);
		std::string s(nodes_[v].depth, '\0');
		for (int i = int(s.size()); i > 0; v = nodes_[v].parent) s[--i] = nodes_[v].letter;
		return s;
	}
	template<class F>
	void for_each_ancestor(int v, F &&visit) const {
		check_(v);
		for (; v != -1; v = nodes_[v].parent) visit(v);
	}
	template<class F>
	void for_each_prefix(std::string_view s, F &&visit) const {
		int v = root;
		if (nodes_[v].terminal) visit(v);
		for (char c : s) {
			v = child(v, c);
			if (v == -1 || !nodes_[v].count) break;
			if (nodes_[v].terminal) visit(v);
		}
	}
	int longest_prefix(std::string_view s) const {
		int v = -1;
		for_each_prefix(s, [&](int u) { v = u; });
		return v;
	}
	int lcp(std::string_view s) const {
		int v = root, length = 0;
		for (char c : s) {
			v = child(v, c);
			if (v == -1 || !nodes_[v].count) break;
			++length;
		}
		return length;
	}
	std::int64_t lcp_sum(std::string_view s) const {
		int v = root;
		std::int64_t sum = 0;
		for (char c : s) {
			v = child(v, c);
			if (v == -1 || !nodes_[v].count) break;
			if (nodes_[v].count > std::numeric_limits<std::int64_t>::max() - sum) throw std::overflow_error("Trie LCP sum");
			sum += nodes_[v].count;
		}
		return sum;
	}
	std::int64_t order_of_key(std::string_view s) const {
		int v = root;
		std::int64_t rank = 0;
		for (char c : s) {
			rank += nodes_[v].terminal;
			int x = symbol_(c);
			for (int j = 0; j < Alphabet && j < x; ++j) {
				int u = nodes_[v].next[j];
				if (u != -1) rank += nodes_[u].count;
			}
			if (x < 0 || x >= Alphabet) break;
			v = nodes_[v].next[x];
			if (v == -1) break;
		}
		return rank;
	}
	int kth_node(std::int64_t k) const {
		if (k < 0 || k >= size()) return -1;
		int v = root;
		while (true) {
			if (k < nodes_[v].terminal) return v;
			k -= nodes_[v].terminal;
			for (int u : nodes_[v].next) if (u != -1) {
				if (k < nodes_[u].count) { v = u; break; }
				k -= nodes_[u].count;
			}
		}
	}
	std::optional<std::string> kth(std::int64_t k) const {
		int v = kth_node(k);
		return v == -1 ? std::nullopt : std::optional<std::string>(restore(v));
	}
	std::optional<std::string> lower_bound(std::string_view s) const { return kth(order_of_key(s)); }
};

#endif
