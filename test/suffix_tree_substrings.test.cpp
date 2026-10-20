#define PROBLEM "https://judge.yosupo.jp/problem/number_of_substrings"
#include "../string/suffix_tree.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string s;
	in.read(s);
	SuffixTree tree(s);
	out.println(tree.distinct_substrings());
}
