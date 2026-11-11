#include "string/suffix_automaton.hpp"
#include <map>
#include <random>
#include <string>

void check(const std::string &s, SuffixAutomaton &sam) {
	std::map<std::string, std::uint64_t> expected;
	expected[""] = s.size() + 1;
	for (int l = 0; l < int(s.size()); ++l) for (int r = l + 1; r <= int(s.size()); ++r) ++expected[s.substr(l, r - l)];
	assert(sam.size() == int(s.size()) && sam.empty() == s.empty());
	assert(sam.distinct_count() == expected.size() - 1);
	assert(sam.node_count() <= std::max(1, int(s.size()) * 2));
	auto counts = sam.occurrences();
	assert(counts == sam.occurrences());
	auto order = sam.order_by_length();
	std::vector<int> rank(sam.node_count());
	for (int i = 0; i < sam.node_count(); ++i) {
		rank[order[i]] = i;
		if (i) assert(sam.length(order[i - 1]) <= sam.length(order[i]));
	}
	for (auto [part, count] : expected) {
		int v = sam.find(part);
		assert(v != -1 && sam.contains(part) && sam.count(part) == count && counts[v] == count);
		assert(sam.min_length(v) <= int(part.size()) && int(part.size()) <= sam.length(v));
		auto [l, r] = sam.first_occurrence(v);
		assert(s.substr(r - part.size(), part.size()) == part);
		assert(s.find(part) == std::size_t(r) - part.size());
		assert(s.substr(l, r - l).size() == std::size_t(sam.length(v)));
	}
	for (int v = 0; v < sam.node_count(); ++v) {
		if (v) assert(rank[sam.suffix_link(v)] < rank[v]);
		std::map<unsigned char, int> edges;
		for (auto [c, to] : sam.transitions(v)) {
			assert(edges.emplace(c, to).second);
			assert(sam.length(v) < sam.length(to) && sam.transition(v, char(c)) == to);
		}
		for (int c = 0; c < 256; ++c) assert(sam.transition(v, char(c)) == (edges.contains(c) ? edges[c] : -1));
	}
	for (int i = 0; i < int(s.size()); ++i) assert(sam.find(std::string_view(s).substr(0, i + 1)) == sam.prefix_nodes()[i]);
}

int main() {
	std::mt19937 rng(727665);
	for (int n = 0; n <= 8; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
		std::string s;
		SuffixAutomaton sam;
		check(s, sam);
		for (int i = 0; i < n; ++i) {
			s += char('a' + (mask >> i & 1));
			assert(sam.push_back(s.back()) == sam.last());
			check(s, sam);
		}
		SuffixAutomaton built(s);
		assert(built.distinct_count() == sam.distinct_count());
		auto copy = sam;
		copy.push_back(char(255));
		check(s + char(255), copy);
		check(s, sam);
		sam.clear(); check("", sam);
	}
	for (int trial = 0; trial < 300; ++trial) {
		std::string s, t;
		for (int n = rng() % 35; n--; ) s += char(rng() % (trial & 1 ? 256 : 4));
		for (int n = rng() % 40; n--; ) t += char(rng() % (trial & 1 ? 256 : 4));
		SuffixAutomaton sam(s);
		check(s, sam);
		int best = 0;
		for (int i = 0; i < int(s.size()); ++i) for (int j = 0; j < int(t.size()); ++j) {
			int k = 0;
			while (i + k < int(s.size()) && j + k < int(t.size()) && s[i + k] == t[j + k]) ++k;
			best = std::max(best, k);
		}
		auto match = sam.longest_common_substring(t);
		assert(match.length == best && s.substr(match.text_pos, best) == t.substr(match.query_pos, best));
		std::string absent(50, char(42));
		assert(!sam.contains(absent) && sam.count(absent) == 0);
	}
	SuffixAutomaton repeated(std::string(200000, 'a'));
	assert(repeated.distinct_count() == 200000 && repeated.count("aaa") == 199998);
	for (int period : {1, 2, 17}) {
		std::string s;
		for (int i = 0; i < 20000; ++i) s += char(i % period);
		SuffixAutomaton periodic(s);
		assert(periodic.node_count() == periodic.size() + 1);
		auto counts = periodic.occurrences();
		auto order = periodic.order_by_length();
		assert(counts[0] == s.size() + 1);
		for (int v = 1; v < periodic.node_count(); ++v) {
			assert(order[v] == v && periodic.length(v) == v);
			assert(counts[v] == (s.size() - v) / period + 1);
		}
	}
	std::string s;
	for (int i = 0; i < 2000; ++i) s += "ab";
	SuffixAutomaton extended(s);
	assert(extended.node_count() == extended.size() + 1 && extended.count("ab") == 2000);
	s += 'b', extended.push_back('b');
	assert(extended.node_count() > extended.size() + 1);
	for (std::string pattern : {std::string{}, std::string("a"), std::string("b"), std::string("ab"),
		std::string("bb"), std::string("ba"), s.substr(0, 129)}) {
		std::uint64_t count = 0;
		for (std::size_t pos = 0; pos <= s.size(); ++pos)
			if (s.compare(pos, pattern.size(), pattern) == 0) ++count;
		assert(extended.count(pattern) == count);
	}
	auto order = extended.order_by_length();
	for (std::size_t i = 1; i < order.size(); ++i)
		assert(extended.length(order[i - 1]) <= extended.length(order[i]));
}
