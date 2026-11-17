#define PROBLEM "https://judge.yosupo.jp/problem/zalgorithm"
#include "string/suffix_tree.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	SuffixTree tree(s);
	for (int i = 0; i < tree.size(); ++i) {
		if (i) output.write(' ');
		output.write(tree.lcp(0, i));
	}
	output.write('\n');
}
