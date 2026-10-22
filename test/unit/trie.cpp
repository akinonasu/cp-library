#include "string/trie.hpp"
#include <algorithm>
#include <map>
#include <numeric>
#include <random>

using Dictionary = std::map<std::string, std::int64_t>;

template<class T>
void structure(const T &trie, const Dictionary &words) {
	std::int64_t size = 0;
	for (const auto &[s, count] : words) size += count;
	assert(trie.size() == size && trie.empty() == words.empty());
	assert(trie.distinct_size() == int(words.size()));
	assert(trie.parent(trie.root) == -1 && trie.depth(trie.root) == 0);
	for (int v = 0; v < trie.node_count(); ++v) {
		std::int64_t count = trie.terminal_count(v);
		for (int u : trie.children(v)) if (u != -1) {
			assert(v < u && u < trie.node_count());
			assert(trie.parent(u) == v && trie.depth(u) == trie.depth(v) + 1);
			assert(trie.child(v, trie.letter(u)) == u);
			count += trie.prefix_count(u);
		}
		assert(count == trie.prefix_count(v));
		if (v) assert(trie.parent(v) < v);
	}
}

template<class T>
void query(const T &trie, const Dictionary &words, const std::string &s) {
	std::int64_t count = 0, prefix = 0, rank = 0, sum = 0;
	int common = 0;
	std::vector<std::string> prefixes;
	std::optional<std::string> lower;
	for (const auto &[word, copies] : words) {
		if (word == s) count = copies;
		if (word.starts_with(s)) prefix += copies;
		if (word < s) rank += copies;
		else if (!lower) lower = word;
		int length = 0;
		while (length < int(word.size()) && length < int(s.size()) && word[length] == s[length]) ++length;
		common = std::max(common, length);
		sum += length * copies;
		if (s.starts_with(word)) prefixes.push_back(word);
	}
	assert(trie.count(s) == count && trie.contains(s) == (count > 0));
	assert(trie.prefix_count(s) == prefix && trie.order_of_key(s) == rank);
	assert(trie.lcp(s) == common && trie.lcp_sum(s) == sum);
	assert(trie.lower_bound(s) == lower);
	int v = trie.find(s);
	assert(trie.walk(trie.root, s) == v);
	if (v != -1) {
		assert(trie.restore(v) == s && trie.depth(v) == int(s.size()));
		assert(trie.terminal_count(v) == count && trie.prefix_count(v) == prefix);
	}
	std::vector<std::string> actual;
	trie.for_each_prefix(s, [&](int u) { actual.push_back(trie.restore(u)); });
	assert(actual == prefixes);
	int longest = trie.longest_prefix(s);
	assert(prefixes.empty() ? longest == -1 : trie.restore(longest) == prefixes.back());
}

template<class T>
void ranks(const T &trie, const Dictionary &words) {
	std::int64_t k = 0;
	for (const auto &[word, copies] : words) for (int i = 0; i < copies; ++i, ++k) {
		assert(trie.kth(k) == word);
		assert(trie.restore(trie.kth_node(k)) == word);
	}
	assert(trie.kth_node(-1) == -1 && trie.kth_node(k) == -1);
	assert(!trie.kth(-1) && !trie.kth(k));
}

void random_test() {
	std::mt19937 rng(8152026);
	Trie<3, 'a', int> trie;
	Dictionary words;
	auto word = [&] {
		std::string s(rng() % 9, 'a');
		for (char &c : s) c += rng() % 3;
		return s;
	};
	for (int step = 0; step < 2500; ++step) {
		std::string s = word();
		int copies = 1 + rng() % 4;
		int operation = rng() % 12;
		if (operation < 5) {
			int old = trie.find(s), v = trie.insert(s, copies);
			if (old != -1) assert(old == v);
			words[s] += copies;
		} else if (operation < 9) {
			auto it = words.find(s);
			bool expected = it != words.end() && it->second >= copies;
			assert(trie.erase(s, copies) == expected);
			if (expected && (it->second -= copies) == 0) words.erase(it);
		} else {
			int old_size = trie.node_count(), v = trie.ensure(s);
			assert(trie.node_count() >= old_size);
			int ancestors = 0;
			trie.for_each_ancestor(v, [&](int u) {
				assert(trie.depth(u) == int(s.size()) - ancestors++);
				++trie.data(u);
			});
			assert(ancestors == int(s.size()) + 1);
			if (operation == 10) {
				trie.insert_node(v, copies);
				words[s] += copies;
				assert(trie.erase_node(v, copies));
				if ((words[s] -= copies) == 0) words.erase(s);
			}
		}
		if (step % 40 == 0) {
			structure(trie, words);
			ranks(trie, words);
		}
		query(trie, words, s);
		query(trie, words, word());
		query(trie, words, "`a");
		query(trie, words, "az");
		query(trie, words, "a`b");
		if (step % 500 == 499) {
			trie.clear(), words.clear();
			assert(trie.node_count() == 1 && trie.data(trie.root) == 0);
			trie.reserve(2000);
		}
	}
}

void prefix_cover() {
	std::vector<std::string> candidates{ "" };
	for (int length = 1; length <= 3; ++length) for (int bits = 0; bits < (1 << length); ++bits) {
		std::string s(length, 'A');
		for (int i = 0; i < length; ++i) s[i] += (bits >> i) & 1;
		candidates.push_back(s);
	}
	std::mt19937 rng(105029);
	for (int trial = 0; trial < 100; ++trial) {
		std::shuffle(candidates.begin(), candidates.end(), rng);
		Trie<2, 'A', std::int64_t> trie;
		std::vector<int> masks;
		for (int i = 0; i < 10; ++i) {
			const auto &s = candidates[i];
			int mask = 0;
			for (int bits = 0; bits < 8; ++bits) {
				std::string leaf(3, 'A');
				for (int j = 0; j < 3; ++j) leaf[j] += (bits >> j) & 1;
				if (leaf.starts_with(s)) mask |= 1 << bits;
			}
			masks.push_back(mask);
			int v = trie.insert(s);
			trie.for_each_ancestor(v, [&](int u) {
				int a = trie.child(u, 'A'), b = trie.child(u, 'B');
				std::int64_t ways = a == -1 || b == -1 ? 0 : trie.data(a) * trie.data(b);
				if (trie.terminal_count(u)) ways += std::int64_t(1) << (trie.prefix_count(u) - 1);
				trie.data(u) = ways;
			});
			std::int64_t expected = 0;
			for (int subset = 0; subset < (1 << int(masks.size())); ++subset) {
				int covered = 0;
				for (int j = 0; j < int(masks.size()); ++j) if ((subset >> j) & 1) covered |= masks[j];
				expected += covered == 255;
			}
			assert(trie.data(trie.root) == expected);
		}
	}
}

int main() {
	Trie<> trie;
	Dictionary words;
	structure(trie, words);
	query(trie, words, "");
	ranks(trie, words);
	int v = trie.ensure("abc");
	assert(v == 3 && trie.empty() && trie.find("abc") == v);
	assert(trie.walk(trie.child(trie.root, 'a'), "bc") == v);
	assert(trie.lcp("abc") == 0 && !trie.erase_node(v));
	trie.insert_node(v, 3);
	assert(trie.erase_node(v, 3) && trie.find("abc") == v);
	assert(trie.insert("abc") == v);
	assert(trie.longest_prefix("abcdef") == v);
	assert(trie.child(trie.root, 'Z') == -1 && trie.find("Z") == -1);
	trie.clear();
	random_test();
	prefix_cover();
	Trie<256, 0> bytes;
	Dictionary binary;
	std::mt19937 rng(842001);
	for (int i = 0; i < 300; ++i) {
		std::string s(rng() % 6, '\0');
		for (char &c : s) c = char(rng() % 256);
		bytes.insert(s);
		++binary[s];
	}
	for (int c = 0; c < 256; ++c) {
		std::string s(1, char(c));
		bytes.insert(s, 2);
		binary[s] += 2;
	}
	structure(bytes, binary);
	ranks(bytes, binary);
	for (const auto &[s, count] : binary) query(bytes, binary, s);
	Trie<10, '0', std::string> digits;
	int id = digits.insert("007");
	digits.data(id) = "payload";
	digits.reserve(1000);
	auto copy = digits;
	assert(copy.data(id) == "payload" && copy.contains("007"));
	assert(digits.erase_node(id) && digits.data(id) == "payload");
	assert(copy.contains("007"));
	digits.clear();
	assert(digits.data(digits.root).empty());
	const std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
	Trie<> bulk;
	bulk.insert("a", maximum);
	assert(bulk.size() == maximum && bulk.lcp_sum("a") == maximum);
	assert(bulk.order_of_key("z") == maximum && bulk.kth(maximum - 1) == "a");
	int nodes = bulk.node_count();
	bool caught = false;
	try { bulk.insert("b"); } catch (const std::overflow_error &) { caught = true; }
	assert(caught && bulk.size() == maximum && bulk.node_count() == nodes);
	assert(bulk.erase("a", maximum) && bulk.empty() && bulk.distinct_size() == 0);
	bulk.insert("aa", maximum / 2 + 1);
	caught = false;
	try { bulk.lcp_sum("aa"); } catch (const std::overflow_error &) { caught = true; }
	assert(caught);
	assert(!bulk.erase("aa", maximum));
	const int length = 500000;
	std::string long_word(length, 'a');
	Trie<2, 'a', int> deep;
	deep.reserve(length);
	int terminal = deep.insert(long_word);
	assert(terminal == length && deep.node_count() == length + 1);
	int visited = 0;
	deep.for_each_ancestor(terminal, [&](int u) { deep.data(u) = ++visited; });
	assert(visited == length + 1 && deep.data(deep.root) == length + 1);
	assert(deep.lcp(long_word) == length && deep.lcp_sum(long_word) == length);
	assert(deep.restore(terminal) == long_word && deep.kth(0) == long_word);
	assert(deep.erase(long_word) && deep.lcp(long_word) == 0);
	assert(deep.insert(long_word) == terminal && deep.node_count() == length + 1);
}
