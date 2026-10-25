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
}
