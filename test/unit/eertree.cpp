#include "string/eertree.hpp"
#include <algorithm>
#include <map>
#include <random>
#include <string>

bool palindrome(std::string_view s) {
	return std::equal(s.begin(), s.begin() + s.size() / 2, s.rbegin());
}

void check(const std::string &s, bool reserve) {
	Eertree tree;
	if (reserve) tree.reserve(static_cast<int>(s.size()));
	assert(tree.empty() && tree.size() == 0 && tree.node_count() == 2 && tree.distinct_count() == 0);
	assert(tree.last() == Eertree::even_root && tree.longest_palindrome() == Eertree::even_root);
	assert(tree.length(0) == -1 && tree.length(1) == 0);
	assert(tree.suffix_link(0) == 0 && tree.suffix_link(1) == 0);
	std::map<std::string, int> id;
	std::vector<std::string> text(2);
	std::vector<std::uint64_t> count(2);
	std::vector<int> first(2), hits(2), prefix;
	std::uint64_t total = 0;
	int longest = 1;
	for (int end = 1; end <= static_cast<int>(s.size()); ++end) {
		int last = -1, suffixes = 0;
		for (int begin = end - 1; begin >= 0; --begin) {
			std::string p = s.substr(begin, end - begin);
			if (!palindrome(p)) continue;
			++suffixes, ++total;
			if (!id.contains(p)) {
				int v = static_cast<int>(text.size());
				id[p] = v;
				text.push_back(p), count.push_back(0), first.push_back(end), hits.push_back(0);
				if (p.size() > text[longest].size()) longest = v;
			}
			last = id.at(p);
			++count[last];
		}
		++hits[last], prefix.push_back(last);
		int v = tree.push_back(s[end - 1]);
		assert(v == last && tree.last() == last);
		assert(!tree.empty() && tree.size() == end && tree.node_count() == static_cast<int>(text.size()));
		assert(tree.distinct_count() == static_cast<int>(id.size()));
		assert(tree.palindrome_count() == total && tree.suffix_count(last) == suffixes);
		assert(tree.longest_palindrome() == longest);
		assert(tree.prefix_nodes() == prefix);
		assert(tree.occurrences() == count);
		assert(tree.occurrences() == count);
		for (int u = 2; u < tree.node_count(); ++u) {
			const auto &p = text[u];
			assert(tree.length(u) == static_cast<int>(p.size()));
			assert(tree.terminal_count(u) == hits[u]);
			assert(tree.first_occurrence(u) == std::make_pair(first[u] - int(p.size()), first[u]));
			int parent = p.size() == 1 ? 0 : p.size() == 2 ? 1 : id.at(p.substr(1, p.size() - 2));
			assert(tree.parent(u) == parent && parent < u);
			assert(tree.transition(parent, p[0]) == u);
			int link = 1, number = 1;
			for (int l = 1; l < static_cast<int>(p.size()); ++l) if (palindrome(std::string_view(p).substr(p.size() - l)))
				link = id.at(p.substr(p.size() - l)), ++number;
			assert(tree.suffix_link(u) == link && link < u);
			assert(tree.suffix_count(u) == number);
		}
	}
	for (int v = 0; v < tree.node_count(); ++v) for (int c = 0; c < 256; ++c) {
		std::string p = v == 0 ? std::string(1, char(c)) : std::string(1, char(c)) + text[v] + char(c);
		auto it = id.find(p);
		assert(tree.transition(v, char(c)) == (it == id.end() ? -1 : it->second));
	}
	Eertree built(s);
	assert(built.prefix_nodes() == prefix && built.occurrences() == count && built.palindrome_count() == total);
	Eertree copy = tree;
	copy.clear();
	assert(copy.empty() && copy.node_count() == 2 && copy.occurrences() == std::vector<std::uint64_t>(2));
	copy.append(s.substr(0, s.size() / 2)), copy.append(s.substr(s.size() / 2));
	assert(copy.prefix_nodes() == tree.prefix_nodes() && copy.occurrences() == count);
}

int main() {
	for (int n = 0; n <= 9; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
		std::string s(n, 'a');
		for (int i = 0; i < n; ++i) s[i] += (mask >> i) & 1;
		check(s, mask % 2);
	}
	std::mt19937 rng(124361);
	for (int i = 0; i < 400; ++i) {
		std::string s(rng() % 55, '\0');
		for (char &c : s) c = char(rng() % (i < 200 ? 3 : 256));
		check(s, i % 2);
	}
	std::string bytes;
	for (int i = 0; i < 256; ++i) bytes += char(i);
	check(bytes + std::string(bytes.rbegin(), bytes.rend()), true);
	Eertree tree;
	tree.reserve(200000);
	for (int i = 1; i <= 200000; ++i) {
		tree.push_back('a');
		assert(tree.distinct_count() == i && tree.length(tree.last()) == i);
		assert(tree.suffix_count(tree.last()) == i);
		assert(tree.palindrome_count() == std::uint64_t(i) * (i + 1) / 2);
	}
	auto count = tree.occurrences();
	for (int v = 2; v < tree.node_count(); ++v) assert(count[v] == std::uint64_t(200001 - tree.length(v)));
	tree.push_back('b');
	assert(tree.length(tree.last()) == 1 && tree.longest_palindrome() == 200001);
}
