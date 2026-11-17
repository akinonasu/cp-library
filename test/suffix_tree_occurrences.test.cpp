#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=ALDS1_14_B"
#include "string/suffix_tree.hpp"
#include "components/fast_io.hpp"
#include <algorithm>

int main() {
	FastInput input;
	FastOutput output;
	std::string text, pattern;
	input.read(text), input.read(pattern);
	SuffixTree tree(text);
	auto positions = tree.occurrences(pattern);
	std::sort(positions.begin(), positions.end());
	for (int position : positions) output.println(position);
}
