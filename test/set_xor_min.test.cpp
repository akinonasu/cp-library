#define PROBLEM "https://judge.yosupo.jp/problem/set_xor_min"
#include "string/trie.hpp"
#include "components/fast_io.hpp"
#include <array>
#include <string_view>

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	Trie<2, '0'> trie;
	while (q--) {
		int type = 0;
		unsigned x = 0;
		input.read(type), input.read(x);
		std::array<char, 30> digits;
		for (int i = 0; i < 30; ++i) digits[i] = '0' + ((x >> (29 - i)) & 1);
		const std::string_view key(digits.data(), digits.size());
		if (type == 0) {
			if (!trie.contains(key)) trie.insert(key);
		} else if (type == 1) trie.erase(key);
		else {
			int node = trie.root;
			unsigned answer = 0;
			for (int i = 0; i < 30; ++i) {
				int next = trie.child(node, digits[i]);
				if (next == -1 || !trie.prefix_count(next)) {
					next = trie.child(node, char(digits[i] ^ 1));
					answer |= 1U << (29 - i);
				}
				node = next;
			}
			output.println(answer);
		}
	}
}
