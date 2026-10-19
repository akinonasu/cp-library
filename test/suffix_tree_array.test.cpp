#define PROBLEM "https://judge.yosupo.jp/problem/suffixarray"
#include "../string/suffix_tree.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string s;
	in.read(s);
	SuffixTree tree(s);
	const auto &sa = tree.suffix_array();
	for (int i = 1; i <= tree.size(); ++i) { if (i != 1) out.write(' '); out.write(sa[i]); }
	out.write('\n');
}
