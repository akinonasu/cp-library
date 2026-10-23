#ifndef SUFFIX_TREE_HPP
#define SUFFIX_TREE_HPP

#include "../components/suffix_array.hpp"
#include "../components/sequence_compare.hpp"
#include "../components/static_range_min.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>

class SuffixTree {
	struct Node { int parent, depth, left, right; };
	std::string text_;
	mutable std::vector<Node> nodes_;
	std::vector<int> sa_;
	mutable std::vector<int> lcp_, rank_;
	mutable std::vector<int> terminals_, branches_, offset_, children_;
	mutable std::vector<unsigned char> symbols_;
	mutable std::array<int, 256> root_children_{};
	mutable bool lcp_ready_ = false, tree_ready_ = false;
	mutable std::uint64_t distinct_ = 0;
	mutable std::optional<StaticRangeMin<std::pair<int, int>>> index_;
	void check([[maybe_unused]] int v) const { prepare_tree(); assert(0 <= v && v < static_cast<int>(nodes_.size())); }
	int range_end(int v) const { int r = nodes_[v].right; return r < 0 ? nodes_[-r - 1].right : r; }
	void prepare_lcp() const {
		if (lcp_ready_) return;
		int n = size();
		rank_.resize(n + 1);
		for (int i = 0; i <= n; ++i) rank_[sa_[i]] = i;
		lcp_.assign(n, 0);
		for (int p = 0, h = 0; p < n; ++p) {
			if (h) --h;
			int i = rank_[p], q = sa_[i - 1];
			h += sequence_compare_detail::prefix(text_.data() + p + h, text_.data() + q + h, std::min(n - p, n - q) - h);
			lcp_[i - 1] = h;
		}
		distinct_ = std::uint64_t(n) * (n + 1) / 2;
		for (int value : lcp_) distinct_ -= value;
		lcp_ready_ = true;
	}
	void prepare_tree() const {
		if (tree_ready_) return;
		prepare_lcp();
		int n = size();
		nodes_.clear();
		nodes_.reserve(std::size_t(n) + 1);
		nodes_.push_back({-1, 0, 0, n + 1});
		terminals_.assign(n + 1, 0);
		branches_.resize(n);
		std::vector<int> stack{0};
		for (int i = 1; i <= n; ++i) {
			int common = lcp_[i - 1], last = -1;
			while (nodes_[stack.back()].depth > common) { last = stack.back(); nodes_[last].right = i; stack.pop_back(); }
			if (nodes_[stack.back()].depth < common) {
				int v = static_cast<int>(nodes_.size());
				nodes_.push_back({stack.back(), common, nodes_[last].left, n + 1});
				nodes_[last].parent = v;
				stack.push_back(v);
			}
			branches_[i - 1] = stack.back();
			int v = static_cast<int>(nodes_.size()), p = sa_[i];
			nodes_.push_back({stack.back(), n - p, i, n + 1});
			terminals_[p] = v;
			stack.push_back(v);
		}
		int count = static_cast<int>(nodes_.size());
		offset_.assign(nodes_.size() + 1, 0);
		for (int v = 1; v < count; ++v) ++offset_[nodes_[v].parent + 1];
		for (int v = 0; v < count; ++v) offset_[v + 1] += offset_[v];
		children_.resize(nodes_.size() - 1);
		symbols_.resize(children_.size());
		auto cursor = offset_;
		for (int v = 1; v < count; ++v) {
			int p = nodes_[v].parent, i = cursor[p]++;
			children_[i] = v;
			symbols_[i] = static_cast<unsigned char>(text_[sa_[nodes_[v].left] + nodes_[p].depth]);
		}
		root_children_.fill(-1);
		for (int i = offset_[0]; i < offset_[1]; ++i) {
			int v = children_[i];
			root_children_[static_cast<unsigned char>(text_[sa_[nodes_[v].left]])] = v;
		}
		for (int i = n; i >= 0; --i) {
			int v = terminals_[sa_[i]];
			if (offset_[v + 1] - offset_[v] == 1) {
				int u = children_[offset_[v]];
				nodes_[v].right = nodes_[u].right < 0 ? nodes_[u].right : -u - 1;
			}
		}
		tree_ready_ = true;
	}
	void prepare_index() const {
		prepare_tree();
		if (index_) return;
		std::vector<std::pair<int, int>> values(size());
		for (int i = 0; i < size(); ++i) values[i] = {lcp_[i], branches_[i]};
		index_.emplace(std::move(values));
	}
	int child(int v, unsigned char c) const {
		if (!v) return root_children_[c];
		int l = offset_[v], r = offset_[v + 1];
		if (r - l <= 4) {
			for (int i = l; i < r; ++i) if (symbols_[i] == c) return children_[i];
			return -1;
		}
		int end = r;
		while (l < r) { int m = l + (r - l) / 2; if (symbols_[m] < c) l = m + 1; else r = m; }
		return l < end && symbols_[l] == c ? children_[l] : -1;
	}

  public:
	struct Locus { int node, length; };
	static constexpr int root = 0;
	SuffixTree() { build({}); }
	explicit SuffixTree(std::string_view s) { build(s); }
	SuffixTree(const SuffixTree &) = default;
	SuffixTree &operator=(const SuffixTree &) = default;
	SuffixTree(SuffixTree &&other) : SuffixTree() { swap(other); }
	SuffixTree &operator=(SuffixTree &&other) {
		if (this != &other) { SuffixTree moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(SuffixTree &other) noexcept {
		text_.swap(other.text_);
		nodes_.swap(other.nodes_);
		sa_.swap(other.sa_); lcp_.swap(other.lcp_); rank_.swap(other.rank_);
		terminals_.swap(other.terminals_); branches_.swap(other.branches_);
		offset_.swap(other.offset_); children_.swap(other.children_);
		symbols_.swap(other.symbols_);
		root_children_.swap(other.root_children_);
		std::swap(distinct_, other.distinct_);
		std::swap(lcp_ready_, other.lcp_ready_);
		std::swap(tree_ready_, other.tree_ready_);
		index_.swap(other.index_);
	}
	void build(std::string_view s) {
		assert(s.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		std::string copy(s);
		text_.swap(copy);
		int n = size();
		sa_ = suffix_array_detail::bytes(text_);
		sa_.insert(sa_.begin(), n);
		rank_.clear(); lcp_.clear();
		lcp_ready_ = false;
		nodes_.clear(); terminals_.clear(); branches_.clear(); offset_.clear(); children_.clear(); symbols_.clear();
		tree_ready_ = false;
		index_.reset();
	}
	void clear() { build({}); }
	int size() const { return static_cast<int>(text_.size()); }
	bool empty() const { return text_.empty(); }
	int node_count() const { prepare_tree(); return static_cast<int>(nodes_.size()); }
	std::string_view text() const { return text_; }
	int parent(int v) const { check(v); return nodes_[v].parent; }
	int depth(int v) const { check(v); return nodes_[v].depth; }
	std::pair<int, int> edge_range(int v) const {
		check(v);
		if (!v) return {0, 0};
		int p = sa_[nodes_[v].left];
		return {p + nodes_[nodes_[v].parent].depth, p + nodes_[v].depth};
	}
	std::pair<int, int> label_range(int v) const { check(v); int p = sa_[nodes_[v].left]; return {p, p + nodes_[v].depth}; }
	std::span<const int> children(int v) const {
		check(v);
		int count = offset_[v + 1] - offset_[v];
		return count ? std::span<const int>(children_.data() + offset_[v], count) : std::span<const int>{};
	}
	int transition(int v, char c) const { check(v); return child(v, static_cast<unsigned char>(c)); }
	int suffix_start(int v) const { check(v); int p = sa_[nodes_[v].left]; return size() - p == nodes_[v].depth ? p : -1; }
	int suffix_node(int p) const { prepare_tree(); assert(0 <= p && p <= size()); return terminals_[p]; }
	std::pair<int, int> occurrence_range(int v) const { check(v); return {nodes_[v].left, range_end(v)}; }
	int occurrence_count(int v) const { check(v); return range_end(v) - nodes_[v].left; }
	const std::vector<int> &suffix_array() const { return sa_; }
	const std::vector<int> &lcp_array() const { prepare_lcp(); return lcp_; }
	std::uint64_t distinct_substrings() const { prepare_lcp(); return distinct_; }
	std::optional<Locus> find(std::string_view pattern) const {
		prepare_tree();
		if (pattern.size() > text_.size()) return std::nullopt;
		int v = 0, matched = 0, length = static_cast<int>(pattern.size());
		while (matched < length) {
			int u = child(v, static_cast<unsigned char>(pattern[matched]));
			if (u == -1) return std::nullopt;
			if (nodes_[u].right >= 0) {
				int start = sa_[nodes_[u].left] + matched;
				int take = std::min(length - matched, nodes_[u].depth - matched);
				if (sequence_compare_detail::prefix(text_.data() + start, pattern.data() + matched, take) != take) return std::nullopt;
				matched += take;
				v = u;
				continue;
			}
			int end = -nodes_[u].right - 1;
			int start = sa_[nodes_[end].left] + nodes_[v].depth;
			int take = std::min(length - matched, nodes_[end].depth - nodes_[v].depth);
			if (sequence_compare_detail::prefix(text_.data() + start, pattern.data() + matched, take) != take) return std::nullopt;
			matched += take;
			if (matched < nodes_[end].depth && nodes_[u].depth < matched) {
				int l = nodes_[u].left + 1, r = std::min(nodes_[end].left, nodes_[u].left + matched - nodes_[u].depth);
				while (l < r) { int m = l + (r - l) / 2; if (size() - sa_[m] < matched) l = m + 1; else r = m; }
				v = l == nodes_[end].left ? end : terminals_[sa_[l]];
			} else v = matched <= nodes_[u].depth ? u : end;
		}
		return Locus{v, length};
	}
	std::optional<Locus> advance(Locus locus, char c) const {
		check(locus.node);
		int v = locus.node;
		assert((!v && locus.length == 0) || (v && nodes_[nodes_[v].parent].depth < locus.length && locus.length <= nodes_[v].depth));
		if (locus.length < nodes_[v].depth) {
			if (text_[sa_[nodes_[v].left] + locus.length] != c) return std::nullopt;
			return Locus{v, locus.length + 1};
		}
		int u = child(v, static_cast<unsigned char>(c));
		return u == -1 ? std::nullopt : std::optional<Locus>(Locus{u, locus.length + 1});
	}
	bool contains(std::string_view pattern) const { return find(pattern).has_value(); }
	int count(std::string_view pattern) const { auto locus = find(pattern); return locus ? occurrence_count(locus->node) : 0; }
	std::vector<int> occurrences(std::string_view pattern) const {
		auto locus = find(pattern);
		if (!locus) return {};
		auto [l, r] = occurrence_range(locus->node);
		return {sa_.begin() + l, sa_.begin() + r};
	}
	int lcp(int p, int q) const {
		assert(0 <= p && p <= size() && 0 <= q && q <= size());
		if (p == q) return size() - p;
		prepare_lcp();
		int a = rank_[p], b = rank_[q];
		if (a > b) std::swap(a, b);
		prepare_index();
		return index_->prod(a, b).first;
	}
	int lca(int u, int v) const {
		check(u); check(v);
		int end_u = range_end(u), end_v = range_end(v);
		if (nodes_[u].left <= nodes_[v].left && end_v <= end_u) return u;
		if (nodes_[v].left <= nodes_[u].left && end_u <= end_v) return v;
		int a = nodes_[u].left, b = nodes_[v].left;
		if (a > b) std::swap(a, b);
		prepare_index();
		return index_->prod(a, b).second;
	}
	int suffix_link(int v) const {
		check(v);
		if (!v) return 0;
		int p = suffix_start(v);
		if (p >= 0) return terminals_[p + 1];
		return lca(terminals_[sa_[nodes_[v].left] + 1], terminals_[sa_[range_end(v) - 1] + 1]);
	}
};

#endif
