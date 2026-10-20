#include "string/suffix_tree.hpp"
#include <random>
#include <set>

int common(std::string_view a, std::string_view b) {
	int n = 0;
	while (n < int(a.size()) && n < int(b.size()) && a[n] == b[n]) ++n;
	return n;
}

void check(const std::string &s) {
	int n = s.size();
	SuffixTree tree(s);
	assert(tree.size() == n && tree.empty() == s.empty() && tree.text() == s);
	assert(tree.node_count() <= 2 * n + 1 && tree.parent(0) == -1 && tree.depth(0) == 0);
	std::vector<int> expected(n + 1);
	std::iota(expected.begin(), expected.end(), 0);
	std::sort(expected.begin(), expected.end(), [&](int p, int q) {
		return std::lexicographical_compare(s.begin() + p, s.end(), s.begin() + q, s.end(),
			[](unsigned char a, unsigned char b) { return a < b; });
	});
	assert(tree.suffix_array() == expected);
	for (int i = 0; i < n; ++i) assert(tree.lcp_array()[i] == common(std::string_view(s).substr(expected[i]), std::string_view(s).substr(expected[i + 1])));
	std::set<std::string> substrings;
	for (int l = 0; l < n; ++l) for (int r = l + 1; r <= n; ++r) substrings.insert(s.substr(l, r - l));
	assert(tree.distinct_substrings() == substrings.size());
	for (int v = 0; v < tree.node_count(); ++v) {
		auto [l, r] = tree.label_range(v);
		std::string label = s.substr(l, r - l);
		assert(r - l == tree.depth(v));
		auto [begin, end] = tree.occurrence_range(v);
		assert(end - begin == tree.occurrence_count(v));
		for (int i = 0; i <= n; ++i) {
			bool present = std::string_view(s).substr(expected[i]).starts_with(label);
			assert(present == (begin <= i && i < end));
		}
		int previous = -1;
		for (int u : tree.children(v)) {
			assert(tree.parent(u) == v && tree.depth(u) > tree.depth(v));
			auto [a, b] = tree.edge_range(u);
			assert(a < b && 0 <= a && b <= n);
			assert(label + s.substr(a, b - a) == s.substr(tree.label_range(u).first, tree.depth(u)));
			unsigned char symbol = s[a];
			assert(previous < symbol && tree.transition(v, char(symbol)) == u);
			previous = symbol;
		}
		int suffix = tree.suffix_start(v);
		if (suffix >= 0) assert(tree.suffix_node(suffix) == v && label == s.substr(suffix));
		int link = tree.suffix_link(v);
		auto [a, b] = tree.label_range(link);
		assert(s.substr(a, b - a) == (label.empty() ? label : label.substr(1)));
	}
	for (int p = 0; p <= n; ++p) for (int q = 0; q <= n; ++q) {
		int length = common(std::string_view(s).substr(p), std::string_view(s).substr(q));
		assert(tree.lcp(p, q) == length);
		assert(tree.depth(tree.lca(tree.suffix_node(p), tree.suffix_node(q))) == length);
	}
	substrings.insert("");
	substrings.insert(s + char(0));
	substrings.insert(std::string(1, char(255)));
	for (int p = 0; p < n; ++p) {
		std::string probe = s.substr(p, std::min(5, n - p));
		probe.back() = char(static_cast<unsigned char>(probe.back()) ^ 128);
		substrings.insert(probe);
	}
	for (const auto &pattern : substrings) {
		std::vector<int> positions;
		for (int p : expected) if (std::string_view(s).substr(p).starts_with(pattern)) positions.push_back(p);
		assert(tree.occurrences(pattern) == positions);
		assert(tree.count(pattern) == int(positions.size()) && tree.contains(pattern) == !positions.empty());
		auto found = tree.find(pattern);
		assert(bool(found) == !positions.empty());
		if (found) assert(found->length == int(pattern.size()) && tree.occurrence_count(found->node) == int(positions.size()));
		std::optional<SuffixTree::Locus> state = SuffixTree::Locus{0, 0};
		for (char c : pattern) if (state) state = tree.advance(*state, c);
		assert(bool(state) == bool(found));
		if (found) assert(state->node == found->node && state->length == found->length);
	}
	if (n <= 16) for (int u = 0; u < tree.node_count(); ++u) for (int v = 0; v < tree.node_count(); ++v) {
		auto [a, b] = tree.label_range(u);
		auto [c, d] = tree.label_range(v);
		assert(tree.depth(tree.lca(u, v)) == common(std::string_view(s).substr(a, b - a), std::string_view(s).substr(c, d - c)));
	}
	auto copy = tree;
	auto moved = SuffixTree(std::move(copy));
	assert(copy.empty() && copy.count("") == 1 && moved.suffix_array() == expected);
	copy = std::move(moved);
	assert(moved.empty() && copy.suffix_array() == expected);
	copy.build(copy.text());
	assert(copy.suffix_array() == expected);
	copy.clear();
	assert(copy.node_count() == 1 && copy.empty() && copy.occurrences("") == std::vector<int>{0});
}

int main() {
	check(""); check("banana"); check("mississippi"); check(std::string("a\0b\0a", 5));
	for (int n = 0; n <= 8; ++n) {
		int patterns = 1;
		for (int i = 0; i < n; ++i) patterns *= 3;
		for (int code = 0; code < patterns; ++code) {
			std::string s(n, 'a');
			for (int i = 0, x = code; i < n; ++i, x /= 3) s[i] += x % 3;
			check(s);
		}
	}
	std::mt19937 rng(20261020);
	for (int n : {15, 16, 17, 31, 32, 33, 63, 64, 65, 99}) for (int t = 0; t < 10; ++t) {
		std::string s(n, 0);
		for (char &c : s) c = char(rng() % (t % 3 == 0 ? 2 : t % 3 == 1 ? 26 : 256));
		check(s);
	}
	std::string bytes;
	for (int c = 0; c < 256; ++c) bytes += char(c);
	check(bytes);
	int n = 200000;
	SuffixTree repeated(std::string(n, 'a'));
	assert(repeated.node_count() == n + 1 && repeated.distinct_substrings() == std::uint64_t(n));
	assert(repeated.count(std::string(100000, 'a')) == n - 99999);
	assert(repeated.lcp(0, 1) == n - 1);
	assert(repeated.suffix_start(repeated.suffix_link(repeated.suffix_node(0))) == 1);
	for (std::string period : {"a", "ab", "aba", "abracadabra"}) {
		std::string s;
		for (int i = 0; i < 4096; ++i) s += period[i % period.size()];
		SuffixTree tree(s);
		for (int i = 0; i < 100; ++i) {
			int p = rng() % s.size(), length = rng() % (s.size() - p + 1);
			std::string pattern = s.substr(p, length);
			if (i % 3 == 0 && !pattern.empty()) pattern.back() ^= 64;
			std::vector<int> expected;
			for (int start : tree.suffix_array()) if (std::string_view(s).substr(start).starts_with(pattern)) expected.push_back(start);
			assert(tree.occurrences(pattern) == expected);
			auto locus = tree.find(pattern);
			std::optional<SuffixTree::Locus> state = SuffixTree::Locus{0, 0};
			for (char c : pattern) if (state) state = tree.advance(*state, c);
			assert(bool(locus) == bool(state));
			if (locus) assert(locus->node == state->node && locus->length == state->length);
		}
	}
	std::string random(n, 0);
	for (char &c : random) c = char(rng());
	SuffixTree large(random);
	for (int i = 0; i < 1000; ++i) {
		int p = rng() % n, q = rng() % n;
		assert(large.lcp(p, q) == common(std::string_view(random).substr(p), std::string_view(random).substr(q)));
	}
}
