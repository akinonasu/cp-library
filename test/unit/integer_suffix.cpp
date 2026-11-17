#include "string/basic_substring_structure.hpp"
#include "string/longest_common_substring.hpp"
#include "components/suffix_range.hpp"
#include <map>
#include <random>
#include <set>
#include <type_traits>

static_assert(std::is_same_v<SuffixTree, BasicSuffixTree<char>>);
static_assert(std::is_same_v<BasicSubstringStructure, SubstringStructure<char>>);
static_assert(std::is_same_v<SuffixRangeIndex, BasicSuffixRangeIndex<char>>);
static_assert(std::is_same_v<SuffixTree::Locus, BasicSuffixTree<std::int64_t>::Locus>);
static_assert(std::is_same_v<decltype(BasicSuffixTree<std::uint64_t>{}.text()), std::span<const std::uint64_t>>);

template <class T>
std::vector<T> slice(const std::vector<T> &s, int l, int r) { return {s.begin() + l, s.begin() + r}; }

template <class T>
int common(std::span<const T> s, std::span<const T> t) {
	int length = 0;
	while (length < int(std::min(s.size(), t.size())) && s[length] == t[length]) ++length;
	return length;
}

template <class T>
void check(const std::vector<T> &s) {
	int n = int(s.size());
	BasicSuffixTree<T> tree(s);
	SuffixLocusIndex locus_index(tree);
	BasicSuffixRangeIndex<T> range_index(s);
	SubstringStructure<T> structure(s);
	assert(tree.size() == n && tree.empty() == s.empty());
	assert(std::equal(s.begin(), s.end(), tree.text().begin(), tree.text().end()));
	assert(std::equal(s.begin(), s.end(), range_index.text().begin(), range_index.text().end()));
	assert(std::equal(s.begin(), s.end(), structure.text().begin(), structure.text().end()));
	std::vector<int> expected(n + 1);
	std::iota(expected.begin(), expected.end(), 0);
	std::sort(expected.begin(), expected.end(), [&](int a, int b) {
		return std::lexicographical_compare(s.begin() + a, s.end(), s.begin() + b, s.end());
	});
	assert(tree.suffix_array() == expected && range_index.suffix_array() == expected);
	for (int i = 0; i < n; ++i) assert(tree.lcp_array()[i] == common<T>(std::span(s).subspan(expected[i]), std::span(s).subspan(expected[i + 1])));
	for (int p = 0; p <= n; ++p) for (int q = 0; q <= n; ++q) {
		int length = common<T>(std::span(s).subspan(p), std::span(s).subspan(q));
		assert(tree.lcp(p, q) == length);
		assert(tree.depth(tree.lca(tree.suffix_node(p), tree.suffix_node(q))) == length);
	}
	std::map<std::vector<T>, int> frequency;
	for (int l = 0; l < n; ++l) for (int r = l + 1; r <= n; ++r) ++frequency[slice(s, l, r)];
	assert(tree.distinct_substrings() == frequency.size() && structure.distinct_substrings() == frequency.size());
	for (int v = 0; v < tree.node_count(); ++v) {
		auto [l, r] = tree.label_range(v);
		auto label = slice(s, l, r);
		auto [a, b] = tree.label_range(tree.suffix_link(v));
		assert(slice(s, a, b) == (label.empty() ? label : slice(label, 1, int(label.size()))));
		std::optional<T> previous;
		for (int u : tree.children(v)) {
			auto [x, y] = tree.edge_range(u);
			assert(x < y && (!previous || *previous < s[x]));
			assert(tree.transition(v, s[x]) == u);
			previous = s[x];
		}
	}
	std::set<std::vector<T>> probes;
	probes.insert(std::vector<T>{});
	probes.insert(std::vector<T>{std::numeric_limits<T>::min()});
	probes.insert(std::vector<T>{std::numeric_limits<T>::max()});
	for (const auto &[value, count] : frequency) {
		probes.insert(value);
		(void)count;
	}
	for (const auto &pattern : probes) {
		std::vector<int> positions;
		for (int p : expected) if (common<T>(std::span(s).subspan(p), std::span(pattern)) == int(pattern.size())) positions.push_back(p);
		assert(tree.occurrences(pattern) == positions);
		assert(tree.count(pattern) == int(positions.size()) && tree.contains(pattern) == !positions.empty());
		auto found = tree.find(pattern);
		std::optional<typename BasicSuffixTree<T>::Locus> state = typename BasicSuffixTree<T>::Locus{0, 0};
		for (T c : pattern) if (state) state = tree.advance(*state, c);
		assert(bool(found) == !positions.empty() && bool(state) == bool(found));
		if (found) assert(state->node == found->node && state->length == found->length);
		auto range = range_index.empty_range();
		for (T c : pattern) range = range_index.concat(range, range_index.character(c));
		assert(range.length == int(pattern.size()) && range.right - range.left == int(positions.size()));
		for (int i = 0; i < int(positions.size()); ++i) assert(expected[range.left + i] == positions[i]);
		auto [prefix, length] = range_index.longest_prefix(pattern);
		int longest = 0;
		for (int p = 0; p <= n; ++p) longest = std::max(longest, common<T>(std::span(s).subspan(p), std::span(pattern)));
		assert(length == longest && prefix.length == longest && prefix.left < prefix.right);
	}
	for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
		auto pattern = std::span(s).subspan(l, r - l);
		auto found = tree.find(pattern);
		auto located = locus_index.locate(l, r);
		assert(found && located.node == found->node && located.length == found->length);
		if (l == r) { assert(!structure.locate(l, r)); continue; }
		auto cell = structure.locate(l, r);
		assert(cell && structure.valid(*cell));
		auto [a, b] = structure.substring_range(*cell);
		assert(slice(s, a, b) == slice(s, l, r));
		assert(structure.occurrence_count(*cell) == frequency.at(slice(s, l, r)));
		assert(structure.erase_first(*cell) == structure.locate(l + 1, r));
		assert(structure.erase_last(*cell) == structure.locate(l, r - 1));
	}
	auto full = range_index.longest_prefix(s).first;
	for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
		auto cut = range_index.substring(full, l, r);
		auto found = tree.find(std::span(s).subspan(l, r - l));
		auto [a, b] = tree.occurrence_range(found->node);
		assert(cut.left == a && cut.right == b && cut.length == r - l);
	}
	auto copy = tree;
	auto moved = BasicSuffixTree<T>(std::move(copy));
	assert(copy.empty() && moved.suffix_array() == expected);
	copy = std::move(moved);
	assert(moved.empty() && copy.suffix_array() == expected);
	copy.build(copy.text());
	assert(copy.suffix_array() == expected);
	copy.clear();
	assert(copy.node_count() == 1 && copy.count({}) == 1);
	auto saved = structure;
	SubstringStructure<T> transferred(std::move(saved));
	assert(saved.empty() && transferred.distinct_substrings() == frequency.size());
	saved = std::move(transferred);
	assert(transferred.empty());
	saved.build(saved.text());
	assert(saved.distinct_substrings() == frequency.size());
	saved.clear();
	assert(saved.empty() && saved.shape() == std::pair(0, 0));
	tree.clear();
	assert(locus_index.size() == n);
	locus_index.build(tree);
	assert(locus_index.size() == 0 && locus_index.locate(0, 0).node == 0);
}

template <class T>
void common_substrings(std::vector<T> s, std::vector<T> t) {
	std::vector<int> dp(t.size() + 1);
	int best = 0;
	for (T c : s) for (int j = int(t.size()); j; --j) {
		dp[j] = c == t[j - 1] ? dp[j - 1] + 1 : 0;
		best = std::max(best, dp[j]);
	}
	auto verify = [&](LongestCommonSubstringResult result) {
		assert(result.length == best);
		assert(slice(s, result.s_pos, result.s_pos + best) == slice(t, result.t_pos, result.t_pos + best));
		if (!best) assert(result.s_pos == 0 && result.t_pos == 0);
	};
	verify(longest_common_substring(s, t));
	verify(longest_common_substring(std::span(s), std::span<const T>(t)));
	verify(longest_common_substring(std::span<const T>(s), std::span(t)));
}

template <class T>
void integer_width() {
	std::vector<T> s{std::numeric_limits<T>::min(), T(7), std::numeric_limits<T>::max(), T(7), T(0)};
	check(s);
	common_substrings(s, slice(s, 1, 4));
	T array[] = {T(7), T(0)};
	const T other[] = {T(1), T(7), T(0)};
	assert(longest_common_substring(std::span(array), std::span(other)).length == 2);
}

void char_sequences() {
	std::vector<char> s{char(255), 'a', char(255), 'a'};
	std::array<char, 2> pattern{char(255), 'a'};
	SuffixTree vector_tree(s), array_tree(pattern), span_tree(std::span<const char>(s).first(2));
	assert(vector_tree.count(pattern) == 2 && vector_tree.contains(std::span(s).first(2)));
	assert(vector_tree.occurrences(pattern) == std::vector<int>({2, 0}));
	assert(vector_tree.find(pattern)->length == 2 && span_tree.count(pattern) == 1);
	vector_tree.build(std::span(s));
	assert(vector_tree.count(pattern) == 2);
	array_tree.build(std::span(pattern));
	assert(array_tree.count(pattern) == 1);
	SuffixRangeIndex index(s);
	assert(index.longest_prefix(pattern).second == 2);
	assert(index.longest_prefix(std::span(s)).second == 4);
	BasicSubstringStructure structure(s);
	assert(structure.locate(0, 2) == structure.locate(2, 4));
	structure.build(std::span(pattern));
	assert(structure.size() == 2 && structure.distinct_substrings() == 3);
	structure.build(std::span(s));
	assert(structure.size() == 4);
}

int main() {
	char_sequences();
	std::vector<SuffixTree> compatible(1);
	std::vector<BasicSubstringStructure> structures(1);
	assert(compatible[0].empty() && structures[0].empty());
	integer_width<signed char>(); integer_width<unsigned char>();
	integer_width<short>(); integer_width<unsigned short>();
	integer_width<int>(); integer_width<unsigned int>();
	integer_width<long>(); integer_width<unsigned long>();
	integer_width<long long>(); integer_width<unsigned long long>();
	for (int n = 0; n <= 7; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
		std::vector<std::int64_t> s(n);
		for (int i = 0; i < n; ++i) s[i] = mask >> i & 1 ? std::numeric_limits<std::int64_t>::min() : std::numeric_limits<std::int64_t>::max();
		check(s);
	}
	std::vector<std::vector<std::int64_t>> words;
	for (int n = 0; n <= 5; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
		std::vector<std::int64_t> s(n);
		for (int i = 0; i < n; ++i) s[i] = mask >> i & 1 ? std::numeric_limits<std::int64_t>::min() : std::numeric_limits<std::int64_t>::max();
		words.push_back(s);
	}
	for (const auto &s : words) for (const auto &t : words) common_substrings(s, t);
	std::mt19937_64 random(7910711);
	for (int trial = 0; trial < 100; ++trial) {
		std::vector<std::uint64_t> s(random() % 35), t(random() % 35);
		for (auto &c : s) c = trial % 2 ? random() : (random() % 4) * (std::uint64_t(1) << 62);
		for (auto &c : t) c = trial % 2 ? random() : (random() % 4) * (std::uint64_t(1) << 62);
		check(s);
		common_substrings(s, t);
	}
	std::vector<std::uint64_t> alphabet(1025);
	for (int i = 0; i < int(alphabet.size()); ++i) alphabet[i] = std::numeric_limits<std::uint64_t>::max() - i;
	BasicSuffixTree<std::uint64_t> tree(alphabet);
	assert(tree.children(0).size() == alphabet.size());
	for (int i = 0; i < int(alphabet.size()); ++i) {
		assert(tree.suffix_array()[i + 1] == int(alphabet.size()) - 1 - i);
		assert(tree.count(std::span(alphabet).subspan(i, 1)) == 1);
	}
	auto reverse = alphabet;
	std::reverse(reverse.begin(), reverse.end());
	assert(longest_common_substring(alphabet, reverse).length == 1);
	std::vector<std::int64_t> repeated(10000, std::numeric_limits<std::int64_t>::min());
	BasicSuffixTree<std::int64_t> chain(repeated);
	assert(chain.count(std::span(repeated).first(5000)) == 5001 && chain.distinct_substrings() == repeated.size());
	SubstringStructure<std::int64_t> structure(repeated);
	assert(structure.distinct_substrings() == repeated.size());
	assert(longest_common_substring(repeated, repeated).length == int(repeated.size()));
	auto shifted = repeated;
	repeated.front() = 12; repeated.back() = 13;
	shifted.front() = 14; shifted.back() = 15;
	assert(longest_common_substring(repeated, shifted).length == int(repeated.size()) - 2);
}
