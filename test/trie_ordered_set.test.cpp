#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#include "string/trie.hpp"
#include "components/fast_io.hpp"
#include <array>
#include <algorithm>

std::array<char, 30> encode(int value) {
	std::array<char, 30> word;
	for (int bit = 0; bit < 30; ++bit) word[bit] = char('0' + ((value >> (29 - bit)) & 1));
	return word;
}

int decode(const std::optional<std::string>& word) {
	if (!word) return -1;
	int value = 0;
	for (char c : *word) value = value * 2 + c - '0';
	return value;
}

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	Trie<2, '0'> trie;
	int capacity = 0;
	for (int depth = 1; depth <= 30; ++depth) capacity += std::min(1 << depth, n + q);
	trie.reserve(capacity);
	for (int i = 0; i < n; ++i) {
		int value = 0;
		input.read(value);
		auto word = encode(value);
		trie.insert_node(trie.ensure(std::string_view(word.data(), word.size())));
	}
	while (q--) {
		int type = 0, value = 0;
		input.read(type), input.read(value);
		auto word = encode(value);
		std::string_view key(word.data(), word.size());
		if (type == 0) { if (!trie.contains(key)) trie.insert(key); }
		if (type == 1) trie.erase(key);
		if (type == 2) output.println(decode(trie.kth(value - 1)));
		if (type == 3) output.println(trie.order_of_key(key) + trie.count(key));
		if (type == 4) {
			auto rank = trie.order_of_key(key) + trie.count(key);
			int node = rank ? trie.kth_node(rank - 1) : -1;
			output.println(node < 0 ? -1 : decode(trie.restore(node)));
		}
		if (type == 5) output.println(decode(trie.lower_bound(key)));
	}
}
