#ifndef SUFFIX_AUTOMATON_HPP
#define SUFFIX_AUTOMATON_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>

class SuffixAutomaton {
	struct Node { int length, link, first_end, hits, head; };
	struct Edge { int to, next; unsigned char symbol; };
	static constexpr int maximum_size = (std::numeric_limits<int>::max() - 1) / 3;
	std::vector<Node> nodes_;
	std::vector<Edge> edges_;
	std::vector<int> prefix_;
	std::array<int, 256> root_edges_;
	mutable std::vector<std::uint64_t> counts_;
	std::uint64_t distinct_ = 0;
	int last_ = 0;
	void check_([[maybe_unused]] int v) const { assert(0 <= v && v < node_count()); }
	int edge_(int v, unsigned char c) const {
		if (!v) return root_edges_[c];
		for (int e = nodes_[v].head; e != -1; e = edges_[e].next)
			if (edges_[e].symbol == c) return e;
		return -1;
	}
	void add_edge_(int v, unsigned char c, int to) {
		int e = static_cast<int>(edges_.size());
		edges_.push_back({to, nodes_[v].head, c});
		nodes_[v].head = e;
		if (!v) root_edges_[c] = e;
	}
	void prepare_counts_() const {
		if (!counts_.empty()) return;
		auto order = order_by_length();
		counts_.resize(nodes_.size());
		for (int v = 0; v < node_count(); ++v) counts_[v] = nodes_[v].hits;
		for (auto it = order.rbegin(); it != order.rend(); ++it)
			if (*it) counts_[nodes_[*it].link] += counts_[*it];
		counts_[root] = std::uint64_t(size()) + 1;
	}

public:
	struct Match { int text_pos = 0, query_pos = 0, length = 0; };
	static constexpr int root = 0;
	SuffixAutomaton() { clear(); }
	explicit SuffixAutomaton(std::string_view s) : SuffixAutomaton() {
		assert(s.size() <= std::size_t(maximum_size));
		reserve(static_cast<int>(s.size()));
		append(s);
	}
	void clear() {
		nodes_.clear(), edges_.clear(), prefix_.clear(), counts_.clear();
		nodes_.push_back({0, -1, 0, 0, -1});
		root_edges_.fill(-1);
		last_ = root, distinct_ = 0;
	}
	void reserve(int n) {
		assert(0 <= n && n <= maximum_size);
		nodes_.reserve(std::size_t(n) * 2 + 1);
		edges_.reserve(std::size_t(n) * 3 + 1);
		prefix_.reserve(n);
	}
	int size() const { return static_cast<int>(prefix_.size()); }
	bool empty() const { return prefix_.empty(); }
	int node_count() const { return static_cast<int>(nodes_.size()); }
	int last() const { return last_; }
	std::uint64_t distinct_count() const { return distinct_; }
	int length(int v) const { check_(v); return nodes_[v].length; }
	int min_length(int v) const { check_(v); return v ? nodes_[nodes_[v].link].length + 1 : 0; }
	int suffix_link(int v) const { check_(v); return nodes_[v].link; }
	std::pair<int, int> first_occurrence(int v) const {
		check_(v);
		return {nodes_[v].first_end - nodes_[v].length, nodes_[v].first_end};
	}
	int transition(int v, char c) const {
		check_(v);
		int e = edge_(v, static_cast<unsigned char>(c));
		return e == -1 ? -1 : edges_[e].to;
	}
	std::vector<std::pair<unsigned char, int>> transitions(int v) const {
		check_(v);
		std::vector<std::pair<unsigned char, int>> result;
		for (int e = nodes_[v].head; e != -1; e = edges_[e].next) result.emplace_back(edges_[e].symbol, edges_[e].to);
		return result;
	}
	int push_back(char symbol) {
		assert(size() < maximum_size);
		counts_.clear();
		unsigned char c = static_cast<unsigned char>(symbol);
		int cur = node_count(), p = last_, e = -1;
		nodes_.push_back({nodes_[last_].length + 1, 0, size() + 1, 1, -1});
		while (p != -1 && (e = edge_(p, c)) == -1) {
			add_edge_(p, c, cur);
			p = nodes_[p].link;
		}
		if (p != -1) {
			int q = edges_[e].to;
			if (nodes_[p].length + 1 == nodes_[q].length) nodes_[cur].link = q;
			else {
				int clone = node_count();
				nodes_.push_back({nodes_[p].length + 1, nodes_[q].link, nodes_[q].first_end, 0, -1});
				for (int at = nodes_[q].head; at != -1; at = edges_[at].next)
					add_edge_(clone, edges_[at].symbol, edges_[at].to);
				while (p != -1 && (e = edge_(p, c)) != -1 && edges_[e].to == q) {
					edges_[e].to = clone;
					p = nodes_[p].link;
				}
				nodes_[q].link = nodes_[cur].link = clone;
			}
		}
		last_ = cur;
		prefix_.push_back(cur);
		distinct_ += nodes_[cur].length - nodes_[nodes_[cur].link].length;
		return cur;
	}
	void append(std::string_view s) {
		assert(s.size() <= std::size_t(maximum_size - size()));
		for (char c : s) push_back(c);
	}
	int walk(int v, std::string_view s) const {
		check_(v);
		for (char c : s) { v = transition(v, c); if (v == -1) break; }
		return v;
	}
	int find(std::string_view s) const { return walk(root, s); }
	bool contains(std::string_view s) const { return find(s) != -1; }
	std::uint64_t count(std::string_view s) const {
		int v = find(s);
		if (v == -1) return 0;
		prepare_counts_();
		return counts_[v];
	}
	std::vector<std::uint64_t> occurrences() const { prepare_counts_(); return counts_; }
	const std::vector<int> &prefix_nodes() const { return prefix_; }
	std::vector<int> order_by_length() const {
		std::vector<int> buckets(std::size_t(size()) + 1), order(nodes_.size());
		for (const auto &node : nodes_) ++buckets[node.length];
		for (int i = 1; i <= size(); ++i) buckets[i] += buckets[i - 1];
		for (int v = node_count() - 1; v >= 0; --v) order[--buckets[nodes_[v].length]] = v;
		return order;
	}
	Match longest_common_substring(std::string_view s) const {
		assert(s.size() <= std::size_t(std::numeric_limits<int>::max()));
		Match best;
		int v = root, matched = 0;
		for (int i = 0; i < static_cast<int>(s.size()); ++i) {
			int next = transition(v, s[i]);
			while (v && next == -1) {
				v = nodes_[v].link;
				matched = nodes_[v].length;
				next = transition(v, s[i]);
			}
			if (next == -1) { v = root; matched = 0; continue; }
			v = next, ++matched;
			if (matched > best.length) best = {nodes_[v].first_end - matched, i + 1 - matched, matched};
		}
		return best;
	}
};

#endif
