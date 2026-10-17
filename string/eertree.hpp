#ifndef EERTREE_HPP
#define EERTREE_HPP

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>

class Eertree {
	struct Node {
		int length, link, parent, first_end, suffix_count, hits, head, next;
	};
	std::vector<Node> nodes_;
	std::vector<std::uint16_t> text_;
	std::vector<int> prefix_;
	std::array<std::array<int, 256>, 2> roots_;
	int last_ = 1, longest_ = 1;
	std::uint64_t total_ = 0;
	int child_(int v, unsigned char c) const {
		if (v < 2) return roots_[v][c];
		for (int u = nodes_[v].head; u; u = nodes_[u].next)
			if (text_[nodes_[u].first_end] == c) return u;
		return 0;
	}
	int extendable_(int v, int position, unsigned char c) const {
		while (text_[position - nodes_[v].length - 1] != c) v = nodes_[v].link;
		return v;
	}
	void check_([[maybe_unused]] int v) const { assert(0 <= v && v < node_count()); }

  public:
	static constexpr int odd_root = 0, even_root = 1;
	Eertree() { clear(); }
	explicit Eertree(std::string_view s) : Eertree() { reserve(static_cast<int>(s.size())); append(s); }
	void clear() {
		nodes_.clear();
		nodes_.push_back({-1, 0, -1, 0, 0, 0, 0, 0});
		nodes_.push_back({0, 0, -1, 0, 0, 0, 0, 0});
		text_.clear(), text_.push_back(256), prefix_.clear();
		roots_[0].fill(0), roots_[1].fill(0);
		last_ = longest_ = even_root, total_ = 0;
	}
	void reserve(int n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() - 2);
		nodes_.reserve(std::size_t(n) + 2), text_.reserve(std::size_t(n) + 1), prefix_.reserve(n);
	}
	int push_back(char symbol) {
		assert(size() < std::numeric_limits<int>::max() - 2);
		unsigned char c = static_cast<unsigned char>(symbol);
		int position = size() + 1;
		text_.push_back(c);
		int parent = extendable_(last_, position, c), v = child_(parent, c);
		if (!v) {
			int link = parent == odd_root ? even_root : child_(extendable_(nodes_[parent].link, position, c), c);
			v = node_count();
			nodes_.push_back({nodes_[parent].length + 2, link, parent, position, nodes_[link].suffix_count + 1, 0, 0, nodes_[parent].head});
			nodes_[parent].head = v;
			if (parent < 2) roots_[parent][c] = v;
			if (nodes_[v].length > nodes_[longest_].length) longest_ = v;
		}
		last_ = v, ++nodes_[v].hits;
		total_ += nodes_[v].suffix_count;
		prefix_.push_back(v);
		return v;
	}
	void append(std::string_view s) { for (char c : s) push_back(c); }
	int size() const { return static_cast<int>(prefix_.size()); }
	bool empty() const { return prefix_.empty(); }
	int node_count() const { return static_cast<int>(nodes_.size()); }
	int distinct_count() const { return node_count() - 2; }
	int last() const { return last_; }
	int longest_palindrome() const { return longest_; }
	std::uint64_t palindrome_count() const { return total_; }
	int length(int v) const { check_(v); return nodes_[v].length; }
	int suffix_link(int v) const { check_(v); return nodes_[v].link; }
	int parent(int v) const { check_(v); assert(v >= 2); return nodes_[v].parent; }
	int suffix_count(int v) const { check_(v); return nodes_[v].suffix_count; }
	int terminal_count(int v) const { check_(v); return nodes_[v].hits; }
	std::pair<int, int> first_occurrence(int v) const {
		check_(v), assert(v >= 2);
		return {nodes_[v].first_end - nodes_[v].length, nodes_[v].first_end};
	}
	int transition(int v, char symbol) const {
		check_(v);
		int u = child_(v, static_cast<unsigned char>(symbol));
		return u ? u : -1;
	}
	const std::vector<int> &prefix_nodes() const { return prefix_; }
	std::vector<std::uint64_t> occurrences() const {
		std::vector<std::uint64_t> count(nodes_.size());
		for (int v = node_count() - 1; v >= 2; --v) {
			count[v] += nodes_[v].hits;
			if (int link = nodes_[v].link; link >= 2) count[link] += count[v];
		}
		return count;
	}
};

#endif
