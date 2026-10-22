#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=ALDS1_4_C"
#include "string/trie.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	Trie<26, 'A'> trie;
	for (int i = 0; i < n; ++i) {
		std::string command, word;
		input.read(command), input.read(word);
		if (command == "insert") {
			if (!trie.contains(word)) trie.insert(word);
		} else output.println(trie.contains(word) ? "yes" : "no");
	}
}
