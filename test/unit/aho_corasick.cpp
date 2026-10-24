#include "string/aho_corasick.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

void require(bool value) { if (!value) std::abort(); }

template <int Alphabet, int First>
void check(const std::vector<std::string> &patterns, const std::string &text, bool reserve) {
	AhoCorasick<Alphabet, First> ac;
	require(!ac.built() && ac.empty() && ac.node_count() == 1);
	int total = 0;
	for (const auto &s : patterns) total += static_cast<int>(s.size());
	if (reserve) ac.reserve(total, static_cast<int>(patterns.size()));
	std::vector<std::string> prefixes(1);
	for (int id = 0; id < static_cast<int>(patterns.size()); ++id) {
		require(ac.add(patterns[id]) == id);
		std::string prefix;
		int v = 0;
		for (char c : patterns[id]) {
			prefix += c;
			int u = ac.child(v, c);
			require(u > 0 && ac.parent(u) == v);
			if (u == static_cast<int>(prefixes.size())) prefixes.push_back(prefix);
			require(prefixes[u] == prefix);
			v = u;
		}
		require(ac.terminal(id) == v && ac.length(id) == static_cast<int>(patterns[id].size()));
	}
	require(ac.size() == static_cast<int>(patterns.size()) && ac.node_count() == static_cast<int>(prefixes.size()));
	ac.build(), ac.build();
	require(ac.built() && ac.empty() == patterns.empty());
	std::vector<int> position(ac.node_count());
	for (int i = 0; i < ac.node_count(); ++i) position[ac.bfs_order()[i]] = i;
	for (int v = 0; v < ac.node_count(); ++v) {
		require(ac.depth(v) == static_cast<int>(prefixes[v].size()));
		int link = 0, output = -1, own = 0, matches = 0;
		for (int u = 0; u < ac.node_count(); ++u) {
			if (prefixes[u].size() >= prefixes[v].size() || !prefixes[v].ends_with(prefixes[u])) continue;
			if (prefixes[u].size() > prefixes[link].size()) link = u;
			if (std::find(patterns.begin(), patterns.end(), prefixes[u]) != patterns.end() &&
				(output == -1 || prefixes[u].size() > prefixes[output].size())) output = u;
		}
		for (const auto &s : patterns) own += s == prefixes[v], matches += prefixes[v].ends_with(s);
		require(ac.suffix_link(v) == link && ac.output_link(v) == output);
		require(ac.terminal_count(v) == own && ac.match_count(v) == matches);
		if (v) require(position[link] < position[v]);
		for (int c = 0; c < 256; ++c) {
			std::string extended = prefixes[v] + static_cast<char>(c);
			int expected = 0, child = -1;
			for (int u = 1; u < ac.node_count(); ++u) {
				if (extended.ends_with(prefixes[u]) && prefixes[u].size() > prefixes[expected].size()) expected = u;
				if (extended == prefixes[u]) child = u;
			}
			require(ac.transition(v, static_cast<char>(c)) == expected && ac.child(v, static_cast<char>(c)) == child);
		}
	}
	std::vector<std::uint64_t> count(patterns.size());
	std::vector<std::pair<std::size_t, int>> expected, actual;
	std::vector<long long> hits(ac.node_count());
	int v = 0;
	for (std::size_t end = 0; end <= text.size(); ++end) {
		if (end) v = ac.transition(v, text[end - 1]);
		++hits[v];
		std::vector<int> suffixes;
		for (int id = 0; id < ac.size(); ++id) {
			const auto &s = patterns[id];
			if (end >= s.size() && text.compare(end - s.size(), s.size(), s) == 0) {
				++count[id], expected.emplace_back(end, id), suffixes.push_back(id);
			}
		}
		std::vector<int> found;
		ac.for_each_match(v, [&](int id) { found.push_back(id); });
		std::sort(found.begin(), found.end());
		require(found == suffixes);
		for (std::size_t split = 0; split <= end; ++split)
			require(ac.walk(ac.walk(0, std::string_view(text).substr(0, split)), std::string_view(text).substr(split, end - split)) == v);
	}
	ac.find_all(text, [&](std::size_t end, int id) { actual.emplace_back(end, id); });
	std::sort(actual.begin(), actual.end());
	require(actual == expected && ac.occurrences(text) == count);
	require(ac.count_matches(text) == std::accumulate(count.begin(), count.end(), std::uint64_t(0)));
	ac.propagate(hits);
	for (int id = 0; id < ac.size(); ++id) require(hits[ac.terminal(id)] == static_cast<long long>(count[id]));
	auto copied = ac;
	ac.clear();
	require(ac.empty() && !ac.built() && ac.node_count() == 1 && ac.child(0, 'a') == -1);
	ac.add("");
	ac.build();
	require(ac.count_matches(text) == text.size() + 1 && ac.occurrences(text)[0] == text.size() + 1);
	require(copied.occurrences(text) == count && copied.count_matches(text) == expected.size());
}

int main() {
	check<26, 'a'>({}, "", false);
	check<26, 'a'>({}, "a!ba", true);
	check<26, 'a'>({"", "", "a", "a", "aa", "baa", "ab", "b"}, "aabaaba!aaa", true);
	check<26, 'a'>({"he", "she", "his", "hers"}, "ushers", false);
	std::vector<std::string> words{""};
	for (int length = 1; length <= 2; ++length)
		for (int mask = 0; mask < (1 << length); ++mask) {
			std::string word(length, 'a');
			for (int i = 0; i < length; ++i) word[i] += (mask >> i) & 1;
			words.push_back(word);
		}
	for (unsigned subset = 0; subset < (1U << words.size()); ++subset) {
		std::vector<std::string> patterns;
		for (int i = 0; i < static_cast<int>(words.size()); ++i) if ((subset >> i) & 1) patterns.push_back(words[i]);
		check<2, 'a'>(patterns, "aabbabbaa", subset & 1);
	}
	std::mt19937 rng(20261020);
	for (int trial = 0; trial < 300; ++trial) {
		std::vector<std::string> patterns(rng() % 12);
		for (auto &s : patterns) {
			s.resize(rng() % 9);
			for (auto &c : s) c = static_cast<char>('a' + rng() % 4);
		}
		std::string text(rng() % 31, ' ');
		for (auto &c : text) c = static_cast<char>('a' + rng() % 5);
		check<4, 'a'>(patterns, text, trial & 1);
	}
	std::vector<std::string> bytes;
	std::string text;
	for (int c = 0; c < 256; ++c) text += static_cast<char>(c);
	for (int c : {0, 1, 127, 128, 254, 255}) {
		bytes.push_back(text.substr(c, 1));
		if (c < 255) bytes.push_back(text.substr(c, 2));
	}
	bytes.push_back("");
	check<256, 0>(bytes, text, true);
	auto widths = [&]<int Alphabet>() {
		std::vector<std::string> patterns;
		for (int c = 0; c < Alphabet; ++c) {
			patterns.push_back(std::string(1, char(c)));
			patterns.push_back(std::string{char(c), char((c + 1) % Alphabet)});
		}
		check<Alphabet, 0>(patterns, text, true);
	};
	widths.template operator()<8>(); widths.template operator()<16>();
	widths.template operator()<31>(); widths.template operator()<32>();
	AhoCorasick<1, 'a'> deep;
	deep.reserve(200000, 3);
	deep.add(std::string(200000, 'a')), deep.add("a"), deep.add("");
	deep.build();
	auto count = deep.occurrences(std::string(200002, 'a'));
	require(count == std::vector<std::uint64_t>({3, 200002, 200003}));
	require(deep.count_matches(std::string(200002, 'a')) == 400008);
	AhoCorasick<> duplicates;
	duplicates.reserve(0, 70000);
	for (int i = 0; i < 70000; ++i) duplicates.add("");
	require(duplicates.terminal_count(0) == 70000);
	duplicates.build();
	require(duplicates.terminal_count(0) == 70000);
	require(duplicates.count_matches(std::string(70000, 'a')) == 4900070000ULL);
	auto repeated = duplicates.occurrences(std::string(70000, 'a'));
	require(std::all_of(repeated.begin(), repeated.end(), [](auto value) { return value == 70001; }));
}
