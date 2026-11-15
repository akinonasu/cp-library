#ifndef SUFFIX_RANGE_HPP
#define SUFFIX_RANGE_HPP

#include "../string/suffix_tree.hpp"

template <class Symbol>
class BasicSuffixRangeIndex {
	using View = std::conditional_t<std::is_same_v<Symbol, char>, std::string_view, std::span<const Symbol>>;
	BasicSuffixTree<Symbol> tree_;
	std::vector<int> rank_, minimum_;
	int capacity_ = 1;
	int first_below(int v, int l, int r, int ql, int qr, int length) const {
		if (r <= ql || qr <= l || minimum_[v] >= length) return -1;
		if (r - l == 1) return l;
		int m = l + (r - l) / 2;
		int a = first_below(v * 2, l, m, ql, qr, length);
		return a == -1 ? first_below(v * 2 + 1, m, r, ql, qr, length) : a;
	}
	int last_below(int v, int l, int r, int ql, int qr, int length) const {
		if (r <= ql || qr <= l || minimum_[v] >= length) return -1;
		if (r - l == 1) return l;
		int m = l + (r - l) / 2;
		int a = last_below(v * 2 + 1, m, r, ql, qr, length);
		return a == -1 ? last_below(v * 2, l, m, ql, qr, length) : a;
	}

  public:
	struct Range { int left = 0, right = 0, length = 0; };
	BasicSuffixRangeIndex() : BasicSuffixRangeIndex(View{}) {}
	explicit BasicSuffixRangeIndex(View text) : tree_(text) {
		int n = tree_.size();
		rank_.resize(n + 1);
		for (int i = 0; i <= n; ++i) rank_[suffix_array()[i]] = i;
		while (capacity_ < n) capacity_ *= 2;
		minimum_.assign(std::size_t(capacity_) * 2, std::numeric_limits<int>::max());
		std::copy(tree_.lcp_array().begin(), tree_.lcp_array().end(), minimum_.begin() + capacity_);
		for (int i = capacity_ - 1; i; --i) minimum_[i] = std::min(minimum_[i * 2], minimum_[i * 2 + 1]);
	}
	template <sequence_view_detail::CharSequence R> requires std::is_same_v<Symbol, char>
	explicit BasicSuffixRangeIndex(const R &text) : BasicSuffixRangeIndex(sequence_view_detail::char_view(text)) {}
	int size() const { return tree_.size(); }
	View text() const { return tree_.text(); }
	const std::vector<int> &suffix_array() const { return tree_.suffix_array(); }
	Range empty_range() const { return {0, size() + 1, 0}; }
	Range character(Symbol c) const {
		int v = tree_.transition(0, c);
		if (v == -1) return {0, 0, 1};
		auto [l, r] = tree_.occurrence_range(v);
		return {l, r, 1};
	}
	Range concat(Range a, Range b) const {
		if (!a.length) return b;
		if (!b.length) return a;
		assert(a.length <= std::numeric_limits<int>::max() - b.length);
		Range missing{0, 0, a.length + b.length};
		if (a.left == a.right || b.left == b.right || a.length > size() - b.length) return missing;
		auto bound = [&](int target, int l) {
			int r = a.right;
			while (l < r) {
				int m = l + (r - l) / 2;
				if (rank_[suffix_array()[m] + a.length] < target) l = m + 1;
				else r = m;
			}
			return l;
		};
		int l = bound(b.left, a.left), r = bound(b.right, l);
		return l == r ? missing : Range{l, r, a.length + b.length};
	}
	Range substring(Range range, int l, int r) const {
		assert(0 <= l && l <= r && r <= range.length && range.left < range.right);
		if (l == r) return empty_range();
		int i = rank_[suffix_array()[range.left] + l], length = r - l;
		int a = last_below(1, 0, capacity_, 0, i, length) + 1;
		int b = first_below(1, 0, capacity_, i, size(), length);
		return {a, b == -1 ? size() + 1 : b + 1, length};
	}
	std::pair<Range, int> longest_prefix(View pattern) const {
		typename BasicSuffixTree<Symbol>::Locus locus{0, 0};
		for (Symbol c : pattern) {
			auto next = tree_.advance(locus, c);
			if (!next) break;
			locus = *next;
		}
		auto [l, r] = tree_.occurrence_range(locus.node);
		return {Range{l, r, locus.length}, locus.length};
	}
	template <sequence_view_detail::CharSequence R> requires std::is_same_v<Symbol, char>
	std::pair<Range, int> longest_prefix(const R &pattern) const { return longest_prefix(sequence_view_detail::char_view(pattern)); }
};

using SuffixRangeIndex = BasicSuffixRangeIndex<char>;

#endif
